#pragma once

#include <gmock/gmock.h>

#include <Compucolor/Debug/IDebugSink.h>

class MockDebugSink: public Compucolor::Debug::IDebugSink
{
    public:
        MOCK_METHOD(bool, IsEnabled, (), (const, override));
        MOCK_METHOD(void, SetCpuState, (const Compucolor::Debug::CpuState&), (override));
        MOCK_METHOD(void, SetCurrentPc, (uint16_t), (override));
        MOCK_METHOD(uint16_t, GetCurrentPc, (), (const, override));
        MOCK_METHOD(void, OnInstruction, (const Compucolor::Debug::InstructionEvent&), (override));
        MOCK_METHOD(void, OnPort, (const Compucolor::Debug::PortEvent&), (override));
        MOCK_METHOD(void, SetTms5501State, (const Compucolor::Debug::Tms5501State&), (override));
        MOCK_METHOD(void, SetSmc5027State, (const Compucolor::Debug::Smc5027State&), (override));
};
