#include "Smc5027Emulator.h"

Smc5027Emulator::Smc5027Emulator():
    _registers(std::vector<uint8_t>(16)) {}

void Smc5027Emulator::Start()
{
    Reset();
}

void Smc5027Emulator::Stop() {}

uint8_t Smc5027Emulator::Read(uint8_t port)
{
    uint8_t convertedPort = ConvertPort(port);
    uint8_t result = 0;

    switch (convertedPort)
    {
        case 0x8:
            result = _registers[0x8];
            break;
        case 0x9:
            result = _registers[0x9];
            break;
    }

    return result;
}

void Smc5027Emulator::Write(uint8_t port, uint8_t data)
{
    uint8_t convertedPort = ConvertPort(port);

    switch (convertedPort)
    {
        case 0x6:
            _registers[0x6] = data;
            break;
        case 0xB:
            _registers[0x6] = (_registers[0x6] + 1) % 32;
            break;
        case 0xC:
            _registers[0x9] = data;
            break;
        case 0xD:
            _registers[0x8] = data & 0x3F;
            break;
    }
}

void Smc5027Emulator::Reset()
{
    for(uint8_t i = 0; i < 16; i++)
        _registers[i] = 0;
}

uint8_t Smc5027Emulator::ConvertPort(uint8_t port)
{
    return port & 0x0F;
}