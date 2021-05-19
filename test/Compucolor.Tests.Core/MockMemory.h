#pragma once

#include <gmock/gmock.h>

#include <Compucolor/Memory/IMemory.h>

class MockMemory: public IMemory
{
    public:
        MOCK_METHOD(uint8_t, GetByte, (uint16_t), (override));
        MOCK_METHOD(void, SetByte, (uint16_t, uint8_t), (override));
};