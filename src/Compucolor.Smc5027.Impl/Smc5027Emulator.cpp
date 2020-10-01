#include "Smc5027Emulator.h"

Smc5027Emulator::Smc5027Emulator():
    _registers(std::vector<uint8_t>(Smc5027Emulator::REGISTER_COUNT))
{
}

void Smc5027Emulator::Start()
{
    Reset();
}

void Smc5027Emulator::Stop()
{
    Reset();
}

uint8_t Smc5027Emulator::Read(uint8_t port)
{
    switch (ConvertPort(port))
    {
        case 0x08:
            return _registers[0x08];

        case 0x09:
            return _registers[0x09];
    }

    return 0x00;
}

void Smc5027Emulator::Write(uint8_t port, uint8_t data)
{
    switch (ConvertPort(port))
    {
        case 0x06:
            _registers[0x06] = data;
            break;
        case 0x0B:
            _registers[0x06] = (_registers[0x06] + 1) % 32;
            break;
        case 0x0C:
            _registers[0x09] = data;
            break;
        case 0x0D:
            _registers[0x08] = data & 0x3F;
            break;
    }
}

void Smc5027Emulator::Reset()
{
    for(
        uint8_t registerIndex = 0;
        registerIndex < Smc5027Emulator::REGISTER_COUNT;
        registerIndex++
    )
    {
        _registers[registerIndex] = 0x00;
    }
}

uint8_t Smc5027Emulator::ConvertPort(uint8_t port)
{
    return port & 0x0F;
}