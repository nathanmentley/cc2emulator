#include "CompucolorEmulator.h"

#include <memory>
#include <utility>

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
    std::shared_ptr<Tms5501::ITms5501Emulator> tms5501
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
    _loop({})
{
    _intel8080->SetBus(std::make_shared<CompucolorIntel8080Bus>(_smc5027, _tms5501));
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

    _loop = _scheduler->SetupRecurringTask(
        502,
        [=, this] {
            // Resets run here so they never race with a CPU step.
            if (const int request = _resetRequest.exchange(ResetNone); request != ResetNone)
            {
                ApplyReset(request == ResetHard);
            }

            if (_paused.load(std::memory_order_relaxed))
            {
                return 0;
            }

            const int steps = _speedRegulated.load(std::memory_order_relaxed) ? 1 : UnregulatedBurst;

            for (int i = 0; i < steps; i++)
            {
                StepOnce();
            }

            return 0;
        }
    );
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::StepOnce()
{
    const bool wasInterruptPending = _intel8080->IsInterruptPending();
    const uint8_t pendingInterrupt = _intel8080->GetPendingInterrupt();

    const uint64_t elapsedCycles = _intel8080->Step();
    _floppyCycleRemainder += elapsedCycles;

    if (wasInterruptPending && !_intel8080->IsInterruptPending()) {
        _tms5501->OnInterruptConsumed(pendingInterrupt);
    }

    _loopInterval++;
    if (_loopInterval % 8 == 0) {
        constexpr uint64_t CPU_CYCLE_NANOSECONDS = 502;
        _tms5501->StepFloppy(
            _floppyCycleRemainder * CPU_CYCLE_NANOSECONDS
        );
        _floppyCycleRemainder = 0;
    }

    if (_loopInterval % 128 == 0) {
        _loopInterval = 0;
        _tms5501->Step();
    }
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::LoadSystemRom()
{
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
