#include "CompucolorEmulator.h"

void LoadRom(IMemory* memory, uint8_t* rom, uint16_t startPosition, uint16_t length)
{
    for(uint16_t i = 0; i < length - 1; i++)
    {
        memory->SetByte(startPosition + i, rom[i]);
    }
}

CompucolorEmulator::CompucolorEmulator(
    std::shared_ptr<ICrtEmulator> crt,
    std::shared_ptr<IIntel8080Emulator> intel8080,
    std::shared_ptr<IKeyboardEmulator> keyboard,
    std::shared_ptr<IMemory> memory,
    std::shared_ptr<ISmc5027Emulator> smc5027,
    std::shared_ptr<ITms5501Emulator> tms5501
):
    _crt(crt),
    _intel8080(intel8080),
    _keyboard(keyboard),
    _memory(memory),
    _smc5027(smc5027),
    _tms5501(tms5501)
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

void CompucolorEmulator::Start()
{
    LoadRom(_memory.get(), get_system_rom_6_78(), 0, get_system_rom_6_78_length());

    _crt->Start();
    _keyboard->Start();
    _smc5027->Start();
    _tms5501->Start();
    _intel8080->Start();
}

void CompucolorEmulator::Stop()
{
    _crt->Stop();
    _keyboard->Stop();
    _smc5027->Stop();
    _tms5501->Stop();
    _intel8080->Stop();
}

void CompucolorEmulator::SetDisplay(IDisplay* display)
{
    _crt->SetDisplay(display);
}

void CompucolorEmulator::OnKeyUp(CompucolorIIKey key)
{
    _keyboard->OnKeyUp(key);
}

void CompucolorEmulator::OnKeyDown(CompucolorIIKey key)
{
    _keyboard->OnKeyDown(key);
}