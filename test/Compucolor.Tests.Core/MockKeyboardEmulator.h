#pragma once

#include <gmock/gmock.h>

#include <Compucolor/Keyboard/IKeyboardEmulator.h>

class MockKeyboardEmulator: public Compucolor::Keyboard::IKeyboardEmulator
{
    public:
        MOCK_METHOD(void, Start, (), (override));
        MOCK_METHOD(void, Stop, (), (override));
        MOCK_METHOD(uint8_t, Read, (uint8_t), (override));
        MOCK_METHOD(void, Reset, (), (override));
        MOCK_METHOD(void, OnKeyUp, (Compucolor::Keyboard::CompucolorIIKey), (override));
        MOCK_METHOD(void, OnKeyDown, (Compucolor::Keyboard::CompucolorIIKey), (override));
};
