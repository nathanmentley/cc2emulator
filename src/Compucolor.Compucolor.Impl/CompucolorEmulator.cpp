#include "CompucolorEmulator.h"

void Compucolor::Compucolor::Impl::LoadRom(Memory::IMemory* memory, uint8_t* rom, uint16_t startPosition, uint16_t length)
{
    for(uint16_t i = 0; i < length - 1; i++)
    {
        memory->SetByte(startPosition + i, rom[i]);
    }
}

Compucolor::Compucolor::Impl::CompucolorEmulator::CompucolorEmulator(
    std::shared_ptr<Crt::ICrtEmulator> crt,
    std::shared_ptr<Intel8080::IIntel8080Emulator> intel8080,
    std::shared_ptr<Keyboard::IKeyboardEmulator> keyboard,
    std::shared_ptr<Memory::IMemory> memory,
    std::shared_ptr<Scheduler::IScheduler> scheduler,
    std::shared_ptr<Smc5027::ISmc5027Emulator> smc5027,
    std::shared_ptr<Tms5501::ITms5501Emulator> tms5501
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
        std::shared_ptr<Compucolor::Impl::CompucolorIntel8080Bus>(
            new Compucolor::Impl::CompucolorIntel8080Bus(
                _smc5027,
                _tms5501
            )
        )
    );
}

void Compucolor::Compucolor::Impl::CompucolorEmulator::Start()
{
    Compucolor::Impl::LoadRom(_memory.get(), get_system_rom_6_78(), 0, get_system_rom_6_78_length());

    _crt->Start();
    _keyboard->Start();
    _smc5027->Start();
    _tms5501->Start();
    _intel8080->Start();

    _loop = _scheduler->SetupReoccuringTask(
        0,//2000,
        [=, this] {
            _intel8080->Step();

            return 0;
        }
    );
}

void Compucolor::Compucolor::Impl::CompucolorEmulator::Stop()
{
    _crt->Stop();
    _keyboard->Stop();
    _smc5027->Stop();
    _tms5501->Stop();
    _intel8080->Stop();
}

void Compucolor::Compucolor::Impl::CompucolorEmulator::SetDisplay(Common::IDisplay* display)
{
    _crt->SetDisplay(display);
}

void Compucolor::Compucolor::Impl::CompucolorEmulator::OnKeyUp(Keyboard::CompucolorIIKey key)
{
    _keyboard->OnKeyUp(key);
}

void Compucolor::Compucolor::Impl::CompucolorEmulator::OnKeyDown(Keyboard::CompucolorIIKey key)
{
    _keyboard->OnKeyDown(key);
}