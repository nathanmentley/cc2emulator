#pragma once

#include <gmock/gmock.h>

#include <Compucolor/Memory/IMemory.h>

class MockMemory: public Compucolor::Memory::IMemory
{
    public:
        MOCK_METHOD(uint8_t, GetByte, (uint16_t), (override));
        MOCK_METHOD(void, SetByte, (uint16_t, uint8_t), (override));

        MOCK_METHOD(void, Start, (), (override));
        MOCK_METHOD(void, Stop, (), (override));
        MOCK_METHOD(std::unique_ptr<boost::json::value>, SaveState, (), (override));
        MOCK_METHOD(void, LoadState, (std::shared_ptr<boost::json::value>), (override));
};