#pragma once

#include <gmock/gmock.h>

#include <Compucolor/Common/IAudioOutput.h>

class MockAudioOutput: public Compucolor::Common::IAudioOutput
{
    public:
        MOCK_METHOD(void, SetLevel, (bool, uint64_t), (override));
        MOCK_METHOD(void, Advance, (uint64_t), (override));
};
