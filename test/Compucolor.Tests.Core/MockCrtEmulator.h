#pragma once

#include <gmock/gmock.h>

#include <Compucolor/Crt/ICrtEmulator.h>

class MockCrtEmulator: public Compucolor::Crt::ICrtEmulator
{
    public:
        MOCK_METHOD(void, Start, (), (override));
        MOCK_METHOD(void, Stop, (), (override));
        MOCK_METHOD(std::unique_ptr<boost::json::value>, SaveState, (), (override));
        MOCK_METHOD(void, LoadState, (std::shared_ptr<boost::json::value>), (override));
        MOCK_METHOD(void, SetLowercaseCharset, (bool), (override));
};
