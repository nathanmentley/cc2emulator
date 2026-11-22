#pragma once

#include <gmock/gmock.h>

#include <Compucolor/Intel8080/IIntel8080Emulator.h>

class MockIntel8080Emulator: public Compucolor::Intel8080::IIntel8080Emulator
{
    public:
        MOCK_METHOD(void, Start, (), (override));
        MOCK_METHOD(void, Stop, (), (override));
        MOCK_METHOD(std::unique_ptr<boost::json::value>, SaveState, (), (override));
        MOCK_METHOD(void, LoadState, (std::shared_ptr<boost::json::value>), (override));
        MOCK_METHOD(void, Reset, (), (override));
        MOCK_METHOD(void, SetBus, (std::shared_ptr<Compucolor::Intel8080::IIntel8080Bus>), (override));
        MOCK_METHOD(void, SetDebugSink, (std::shared_ptr<Compucolor::Debug::IDebugSink>), (override));
        MOCK_METHOD(Compucolor::Debug::CpuState, GetState, (), (const, override));
        MOCK_METHOD(void, RegisterInterrupt, (uint8_t), (override));
        MOCK_METHOD(bool, IsInterruptPending, (), (override));
        MOCK_METHOD(uint8_t, GetPendingInterrupt, (), (override));
        MOCK_METHOD(uint64_t, Step, (), (override));
};
