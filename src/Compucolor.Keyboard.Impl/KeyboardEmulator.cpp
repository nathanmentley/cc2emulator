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
}

#include <iostream>

uint8_t KeyboardEmulator::Read(uint8_t port)
{
    std::cout << "READ FROM KEYBOARd.\n";
    return 0xFF;
}

void KeyboardEmulator::Reset()
{
    for(uint8_t i = 0; i < 17; i++)
    {
        _kbMatrix[i] = 0xFF;
    }
}