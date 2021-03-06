#include "CompucolorEmulator.h"

void Compucolor::Impl::CompucolorII::LoadRom(Compucolor::Memory::IMemory* memory, uint8_t* rom, uint16_t startPosition, uint16_t length)
{
    for(uint16_t i = 0; i < length - 1; i++)
    {
        memory->SetByte(startPosition + i, rom[i]);
    }
}

Compucolor::Impl::CompucolorII::CompucolorEmulator::CompucolorEmulator(
    std::shared_ptr<Compucolor::Crt::ICrtEmulator> crt,
    std::shared_ptr<Compucolor::Intel8080::IIntel8080Emulator> intel8080,
    std::shared_ptr<Compucolor::Keyboard::IKeyboardEmulator> keyboard,
    std::shared_ptr<Compucolor::Memory::IMemory> memory,
    std::shared_ptr<Compucolor::Scheduler::IScheduler> scheduler,
    std::shared_ptr<Compucolor::Smc5027::ISmc5027Emulator> smc5027,
    std::shared_ptr<Compucolor::Tms5501::ITms5501Emulator> tms5501
):
    _crt(crt),
    _intel8080(intel8080),
    _keyboard(keyboard),
    _memory(memory),
    _scheduler(scheduler),
    _smc5027(smc5027),
    _tms5501(tms5501),
    _loop({})
{
    _intel8080->SetBus(
        std::shared_ptr<CompucolorIntel8080Bus>(
            new CompucolorIntel8080Bus(
                _smc5027,
                _tms5501
            )
        )
    );
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::Start()
{
    LoadRom(_memory.get(), get_system_rom_8_79(), 0, get_system_rom_8_79_length());

    _crt->Start();
    _keyboard->Start();
    _smc5027->Start();
    _tms5501->Start();
    _intel8080->Start();

    _loop = _scheduler->SetupReoccuringTask(
        2000,
        [=, this] {
            _intel8080->Step();

            return 0;
        }
    );
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::Stop()
{
    _crt->Stop();
    _keyboard->Stop();
    _smc5027->Stop();
    _tms5501->Stop();
    _intel8080->Stop();
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::OnKeyUp(Compucolor::Keyboard::CompucolorIIKey key)
{
    _keyboard->OnKeyUp(key);
}

void Compucolor::Impl::CompucolorII::CompucolorEmulator::OnKeyDown(Compucolor::Keyboard::CompucolorIIKey key)
{
    _keyboard->OnKeyDown(key);
}