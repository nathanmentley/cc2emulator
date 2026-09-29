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
    std::shared_ptr<Intel8080::IIntel8080Emulator> intel8080,
    std::shared_ptr<Keyboard::IKeyboardEmulator> keyboard,
    std::shared_ptr<Memory::IMemory> memory,
    std::shared_ptr<Scheduler::IScheduler> scheduler,
    std::shared_ptr<Smc5027::ISmc5027Emulator> smc5027,
    std::shared_ptr<Tms5501::ITms5501Emulator> tms5501
):
    _crt(std::move(crt)),
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
    LoadRom(_memory.get(), get_system_rom_8_79(), 0, get_system_rom_8_79_length());

    _crt->Start();
    _keyboard->Start();
    _smc5027->Start();
    _tms5501->Start();
    _intel8080->Start();

    _loop = _scheduler->SetupRecurringTask(
        502,
        [=, this] {
            const bool wasInterruptPending = _intel8080->IsInterruptPending();
            const uint8_t pendingInterrupt = _intel8080->GetPendingInterrupt();

            _intel8080->Step();

            if (wasInterruptPending && !_intel8080->IsInterruptPending()) {
                _tms5501->OnInterruptConsumed(pendingInterrupt);
            }

            _loopInterval++;
            if (_loopInterval % 128 == 0) {
                _loopInterval = 0;

                _tms5501->Step();
            }

            return 0;
        }
    );
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
        manager->GetIntel8080(),
        manager->GetKeyboard(),
        manager->GetMemory(),
        manager->GetScheduler(),
        manager->GetSmc5027(),
        manager->GetTms5501()
    );
}
