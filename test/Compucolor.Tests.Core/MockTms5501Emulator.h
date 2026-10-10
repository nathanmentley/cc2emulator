#pragma once

#include <gmock/gmock.h>

#include <Compucolor/Tms5501/ITms5501Emulator.h>

class MockTms5501Emulator: public Compucolor::Tms5501::ITms5501Emulator
{
    public:
        MOCK_METHOD(void, Start, (), (override));
        MOCK_METHOD(void, Stop, (), (override));
        MOCK_METHOD(uint8_t, Read, (uint8_t), (override));
        MOCK_METHOD(void, Write, (uint8_t, uint8_t), (override));
        MOCK_METHOD(void, Reset, (), (override));
        MOCK_METHOD(void, Step, (), (override));
        MOCK_METHOD(void, StepFloppy, (uint64_t), (override));
        MOCK_METHOD(void, TriggerExternalSensor, (), (override));
        MOCK_METHOD(void, OnInterruptConsumed, (uint8_t), (override));
};
