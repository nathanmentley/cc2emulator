
#include "CompucolorEmulator.h"

void LoadRom(IMemory* memory, uint8_t* rom, uint16_t startPosition, uint16_t length)
{
    for(uint16_t i = 0; i < length - 1; i++)
    {
        memory->SetByte(startPosition + i, rom[i]);
    }
}

CompucolorEmulator::CompucolorEmulator(
    std::shared_ptr<IMemory> memory,
    std::shared_ptr<ICrtEmulator> crt,
    std::shared_ptr<IIntel8080Emulator> intel8080
):
    _memory(memory),
    _crt(crt),
    _intel8080(intel8080)
{
}

void CompucolorEmulator::Start()
{
    LoadRom(_memory.get(), get_system_rom_6_78(), 0, get_system_rom_6_78_length());

    _crt->Start();
    _intel8080->Start();
}

void CompucolorEmulator::Stop()
{
    _crt->Stop();
    _intel8080->Stop();
}

void CompucolorEmulator::SetDisplay(IDisplay* display)
{
    _crt->SetDisplay(display);
}