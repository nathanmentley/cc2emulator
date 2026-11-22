#include "CompucolorEmulator.h"

#include <algorithm>
#include <chrono>
#include <memory>
#include <utility>
#include <fstream>
#include <iterator>
#include <string>
#include <stdexcept>
#include <boost/json.hpp>

void Compucolor::Impl::CompucolorII::LoadRom(
    Memory::IMemory* memory,
    const uint8_t* rom,
    const uint16_t startPosition,
    const uint16_t length
) {
    for(uint16_t i = 0; i < length; i++)
    {
        memory->SetByte(startPosition + i, rom[i]);
    }
}

Compucolor::Impl::CompucolorII::CompucolorEmulator::CompucolorEmulator(
    std::shared_ptr<Crt::ICrtEmulator> crt,
    std::shared_ptr<Floppy::IFloppyEmulator> floppyA,
    std::shared_ptr<Floppy::IFloppyEmulator> floppyB,
    std::shared_ptr<Intel8080::IIntel8080Emulator> intel8080,
    std::shared_ptr<Keyboard::IKeyboardEmulator> keyboard,
    std::shared_ptr<Memory::IMemory> memory,
    std::shared_ptr<Scheduler::IScheduler> scheduler,
    std::shared_ptr<Smc5027::ISmc5027Emulator> smc5027,
    std::shared_ptr<Tms5501::ITms5501Emulator> tms5501,
    std::function<uint64_t()> clock
):
    _crt(std::move(crt)),
    _floppyA(std::move(floppyA)),
    _floppyB(std::move(floppyB)),
    _intel8080(std::move(intel8080)),
    _keyboard(std::move(keyboard)),
    _memory(std::move(memory)),
    _scheduler(std::move(scheduler)),
    _smc5027(std::move(smc5027)),
    _tms5501(std::move(tms5501)),
    _bus({}),
    _debugSink({}),
    _loop({}),
    _clock(std::move(clock))
{
    if (!_clock)
    {
        _clock = [] {
            return static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count()
            );
        };
    }

    _bus = std::make_shared<CompucolorIntel8080Bus>(_smc5027, _tms5501);
    _intel8080->SetBus(_bus);
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::Start()
{
    _loopInterval = 0;
    _floppyCycleRemainder = 0;
    LoadSystemRom();

    _crt->Start();
    _keyboard->Start();
    _smc5027->Start();
    _tms5501->Start();
    _intel8080->Start();

    _lastTickNanoseconds = _clock();
    _budgetNanoseconds = 0;

    _loop = _scheduler->SetupRecurringTask(
        TickNanoseconds,
        [=, this] {
            // Resets run here so they never race with a CPU step.
            if (const int request = _resetRequest.exchange(ResetNone); request != ResetNone)
            {
                ApplyReset(request == ResetHard);
            }

            // The UI reads snapshots from the sink, never live device state.
            auto publishDebugState = [this] {
                if (!_debugSink)
                {
                    return;
                }

                auto state = _intel8080->GetState();
                state.emulatedNanoseconds = _emulatedNanoseconds;
                _debugSink->SetCpuState(state);
                _debugSink->SetTms5501State(_tms5501->GetDebugState());
                _debugSink->SetSmc5027State(_smc5027->GetDebugState());
            };

            const uint64_t now = _clock();
            const uint64_t wallNanoseconds = now - _lastTickNanoseconds;
            _lastTickNanoseconds = now;

            // Time spent paused must not turn into a burst of catch-up work on resume.
            if (_paused.load(std::memory_order_relaxed))
            {
                _budgetNanoseconds = 0;
                publishDebugState();
                return 0;
            }

            if (_speedRegulated.load(std::memory_order_relaxed))
            {
                // Run as many CPU cycles as the wall clock has allowed since the last tick.
                _budgetNanoseconds = std::min(
                    _budgetNanoseconds + static_cast<int64_t>(wallNanoseconds),
                    MaxBudgetNanoseconds
                );

                while (_budgetNanoseconds > 0)
                {
                    _budgetNanoseconds -= static_cast<int64_t>(StepOnce() * CycleNanoseconds);
                }
            }
            else
            {
                _budgetNanoseconds = 0;

                for (int i = 0; i < UnregulatedBurst; i++)
                {
                    StepOnce();
                }
            }

            publishDebugState();

            return 0;
        }
    );
}

uint64_t Compucolor::Impl::CompucolorII::CompucolorEmulator::StepOnce()
{
    const bool wasInterruptPending = _intel8080->IsInterruptPending();
    const uint8_t pendingInterrupt = _intel8080->GetPendingInterrupt();

    // Peripherals stamp events, such as audio edges, with the time this instruction starts.
    _tms5501->SetEmulatedTime(_emulatedNanoseconds);

    // A step always takes at least one cycle, which also keeps the pacing loop finite.
    const uint64_t elapsedCycles = std::max<uint64_t>(_intel8080->Step(), 1);
    _emulatedNanoseconds += elapsedCycles * CycleNanoseconds;
    _floppyCycleRemainder += elapsedCycles;

    if (wasInterruptPending && !_intel8080->IsInterruptPending()) {
        _tms5501->OnInterruptConsumed(pendingInterrupt);
    }

    _loopInterval++;
    if (_loopInterval % 8 == 0) {
        _tms5501->StepFloppy(
            _floppyCycleRemainder * CycleNanoseconds
        );
        _floppyCycleRemainder = 0;
    }

    if (_loopInterval % 128 == 0) {
        _loopInterval = 0;
        _tms5501->Step();
    }

    return elapsedCycles;
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::LoadSystemRom()
{
    // The drive hardware changed with the ROM: v6.78 has a 3 phase stepper, v8.79 a 4 phase one.
    const int stepperPhases = _romVersion.load() == RomVersion::V6_78 ? 3 : 4;
    _floppyA->SetStepperPhases(stepperPhases);
    _floppyB->SetStepperPhases(stepperPhases);

    if (_romVersion.load() == RomVersion::V6_78)
    {
        LoadRom(_memory.get(), get_system_rom_6_78(), 0, get_system_rom_6_78_length());
    }
    else
    {
        LoadRom(_memory.get(), get_system_rom_8_79(), 0, get_system_rom_8_79_length());
    }
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::ApplyReset(const bool hard)
{
    _intel8080->Reset();
    _tms5501->Reset();
    _smc5027->Reset();
    _keyboard->Reset();

    _loopInterval = 0;
    _floppyCycleRemainder = 0;

    if (hard)
    {
        LoadSystemRom();
        // BASIC treats 0x97 here as a warm start; clearing it forces a cold start.
        _memory->SetByte(PowerUpFlagAddress, 0x00);
    }
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::Reset(const bool hard)
{
    _resetRequest.store(hard ? ResetHard : ResetWarm);
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::Pause()
{
    _paused.store(true, std::memory_order_relaxed);
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::Resume()
{
    _paused.store(false, std::memory_order_relaxed);
}

bool Compucolor::Impl::CompucolorII::CompucolorEmulator::IsPaused() const
{
    return _paused.load(std::memory_order_relaxed);
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::SetSpeedRegulated(const bool regulated)
{
    _speedRegulated.store(regulated, std::memory_order_relaxed);
}

bool Compucolor::Impl::CompucolorII::CompucolorEmulator::IsSpeedRegulated() const
{
    return _speedRegulated.load(std::memory_order_relaxed);
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::SetLowercaseCharset(const bool lowercase)
{
    _crt->SetLowercaseCharset(lowercase);
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::SetRomVersion(const RomVersion version)
{
    _romVersion.store(version);
    Reset(true);
}

std::unique_ptr<boost::json::value> Compucolor::Impl::CompucolorII::CompucolorEmulator::SaveState()
{
    const bool wasPaused = IsPaused();
    Pause();

    try
    {
        boost::json::object state;

        auto saveComponent = [](const auto& component) -> boost::json::value
        {
            if (!component)
            {
                throw std::runtime_error("Cannot save null emulator component");
            }

            auto componentState = component->SaveState();

            if (!componentState)
            {
                throw std::runtime_error("Component returned an empty save state");
            }

            return *componentState;
        };

        state["crt"] = saveComponent(_crt);
        state["floppyA"] = saveComponent(_floppyA);
        state["floppyB"] = saveComponent(_floppyB);
        state["intel8080"] = saveComponent(_intel8080);
        state["keyboard"] = saveComponent(_keyboard);
        state["memory"] = saveComponent(_memory);
        state["smc5027"] = saveComponent(_smc5027);
        state["tms5501"] = saveComponent(_tms5501);

        // Emulator scheduling state.
        state["loopInterval"] = static_cast<int64_t>(_loopInterval);
        state["floppyCycleRemainder"] = _floppyCycleRemainder;
        state["emulatedNanoseconds"] = _emulatedNanoseconds;

        // Machine configuration.
        state["romVersion"] =
            _romVersion.load() == RomVersion::V6_78 ? "6.78" : "8.79";

        state["speedRegulated"] = _speedRegulated.load();

        // Preserve a reset request that hasn't yet been processed.
        state["resetRequest"] =
            static_cast<int64_t>(_resetRequest.load());

        auto result = std::make_unique<boost::json::value>(
            std::move(state)
        );

        if (!wasPaused)
        {
            Resume();
        }

        return result;
    }
    catch (...)
    {
        if (!wasPaused)
        {
            Resume();
        }

        throw;
    }
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::LoadState(
    std::shared_ptr<boost::json::value> value
)
{
    const bool wasPaused = IsPaused();
    Pause();

    try
    {
        if (!value || !value->is_object())
        {
            throw std::runtime_error("Invalid Compucolor emulator state");
        }

        const auto& state = value->as_object();

        auto getUnsigned = [](const boost::json::value& value) -> uint64_t
        {
            if (value.is_uint64())
            {
                return value.as_uint64();
            }

            if (value.is_int64() && value.as_int64() >= 0)
            {
                return static_cast<uint64_t>(value.as_int64());
            }

            throw std::runtime_error(
                "Expected non-negative integer in Compucolor state"
            );
        };

        const int64_t loopIntervalValue =
            state.at("loopInterval").as_int64();

        if (loopIntervalValue < 0 || loopIntervalValue >= 128)
        {
            throw std::runtime_error("Invalid loop interval in Compucolor state");
        }

        const uint64_t floppyCycleRemainder =
            getUnsigned(state.at("floppyCycleRemainder"));

        const uint64_t emulatedNanoseconds =
            getUnsigned(state.at("emulatedNanoseconds"));

        const std::string romVersion(
            state.at("romVersion").as_string().c_str()
        );

        RomVersion restoredRomVersion;

        if (romVersion == "6.78")
        {
            restoredRomVersion = RomVersion::V6_78;
        }
        else if (romVersion == "8.79")
        {
            restoredRomVersion = RomVersion::V8_79;
        }
        else
        {
            throw std::runtime_error("Unsupported ROM version in Compucolor state");
        }

        const bool speedRegulated =
            state.at("speedRegulated").as_bool();

        const int64_t resetRequestValue =
            state.at("resetRequest").as_int64();

        if (resetRequestValue < ResetNone || resetRequestValue > ResetHard)
        {
            throw std::runtime_error("Invalid reset request in Compucolor state");
        }

        // Restore component states.
        //
        // Memory must be restored before the CPU resumes execution.
        _memory->LoadState(
            std::make_shared<boost::json::value>(state.at("memory"))
        );

        _floppyA->LoadState(
            std::make_shared<boost::json::value>(state.at("floppyA"))
        );

        _floppyB->LoadState(
            std::make_shared<boost::json::value>(state.at("floppyB"))
        );

        _keyboard->LoadState(
            std::make_shared<boost::json::value>(state.at("keyboard"))
        );

        _crt->LoadState(
            std::make_shared<boost::json::value>(state.at("crt"))
        );

        _smc5027->LoadState(
            std::make_shared<boost::json::value>(state.at("smc5027"))
        );

        _intel8080->LoadState(
            std::make_shared<boost::json::value>(state.at("intel8080"))
        );

        _tms5501->LoadState(
            std::make_shared<boost::json::value>(state.at("tms5501"))
        );

        // Restore emulator scheduling state.
        _loopInterval = static_cast<int>(loopIntervalValue);
        _floppyCycleRemainder = floppyCycleRemainder;
        _emulatedNanoseconds = emulatedNanoseconds;

        // Restore machine configuration.
        _romVersion.store(restoredRomVersion);
        _speedRegulated.store(speedRegulated);
        _resetRequest.store(static_cast<int>(resetRequestValue));

        // Wall-clock pacing state must be restarted from the current time.
        _lastTickNanoseconds = _clock();
        _budgetNanoseconds = 0;

        if (!wasPaused)
        {
            Resume();
        }
    }
    catch (...)
    {
        if (!wasPaused)
        {
            Resume();
        }

        throw;
    }
}

std::shared_ptr<Compucolor::Floppy::IFloppyEmulator> Compucolor::Impl::CompucolorII::CompucolorEmulator::GetFloppy(
    const int drive
) const
{
    switch (drive)
    {
        case 0: return _floppyA;
        case 1: return _floppyB;
        default: return nullptr;
    }
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::SetDebugSink(
    std::shared_ptr<Compucolor::Debug::IDebugSink> sink
)
{
    _debugSink = std::move(sink);

    _intel8080->SetDebugSink(_debugSink);
    _bus->SetDebugSink(_debugSink);
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::Stop()
{
    if (_loop)
    {
        _loop->Stop();
        _loop->Join();
        _loop.reset();
    }

    _crt->Stop();
    _keyboard->Stop();
    _smc5027->Stop();
    _tms5501->Stop();
    _intel8080->Stop();
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::OnKeyUp(Keyboard::CompucolorIIKey key)
{
    _keyboard->OnKeyUp(key);
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::OnKeyDown(Keyboard::CompucolorIIKey key)
{
    _keyboard->OnKeyDown(key);
}

std::shared_ptr<Compucolor::ICompucolorEmulator> Compucolor::Impl::CompucolorII::CompucolorEmulator::Creator(
    Common::IPluginManager* manager
)
{
    return std::make_shared<CompucolorEmulator>(
        manager->GetCrt(),
        manager->GetFloppyA(),
        manager->GetFloppyB(),
        manager->GetIntel8080(),
        manager->GetKeyboard(),
        manager->GetMemory(),
        manager->GetScheduler(),
        manager->GetSmc5027(),
        manager->GetTms5501()
    );
}
