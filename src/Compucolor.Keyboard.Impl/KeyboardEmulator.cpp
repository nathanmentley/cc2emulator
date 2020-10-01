#include "KeyboardEmulator.h"

KeyboardEmulator::KeyboardEmulator():
    _kbMatrix(std::vector<uint8_t>(17))
{
}

void KeyboardEmulator::Start()
{
    Reset();
}

void KeyboardEmulator::Stop()
{
    Reset();
}

#include <iostream>

uint8_t KeyboardEmulator::Read(uint8_t port)
{
    printf("READ ROW %d from keyboard\n", port);

    if (port == 14)
    {
        return 0xFD;
    }

    return _kbMatrix[port];

}

void KeyboardEmulator::Reset()
{
    for(uint8_t i = 0; i < 17; i++)
    {
        _kbMatrix[i] = 0xFF;
    }
}

void KeyboardEmulator::OnKeyUp(CompucolorIIKey key)
{
    _kbMatrix[14] = 0xFF;
}

void KeyboardEmulator::OnKeyDown(CompucolorIIKey key)
{
    _kbMatrix[14] = 0xFD;
}