#pragma once

#include <gmock/gmock.h>

#include <Compucolor.Smc5027/ISmc5027Emulator.h>

class MockSmc5027Emulator: public ISmc5027Emulator
{
    public:
        MOCK_METHOD(void, Start, (), (override));
        MOCK_METHOD(void, Stop, (), (override));
        MOCK_METHOD(uint8_t, Read, (uint8_t), (override));
        MOCK_METHOD(void, Write, (uint8_t, uint8_t), (override));
        MOCK_METHOD(void, Reset, (), (override));
        MOCK_METHOD(uint8_t, GetCursorX, (), (override));
        MOCK_METHOD(uint8_t, GetCursorY, (), (override));
};