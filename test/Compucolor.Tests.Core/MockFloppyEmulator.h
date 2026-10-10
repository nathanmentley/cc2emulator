#pragma once

#include <string>

#include <gmock/gmock.h>

#include <Compucolor/Floppy/IFloppyEmulator.h>

class MockFloppyEmulator: public Compucolor::Floppy::IFloppyEmulator
{
    public:
        MOCK_METHOD(void, Start, (), (override));
        MOCK_METHOD(void, Stop, (), (override));
        MOCK_METHOD(uint8_t, Read, (uint8_t), (override));
        MOCK_METHOD(void, Write, (uint8_t, uint8_t), (override));
        MOCK_METHOD(void, Reset, (), (override));
        MOCK_METHOD(void, Select, (bool, bool, uint8_t), (override));
        MOCK_METHOD(void, Step, (uint64_t), (override));
        MOCK_METHOD(bool, HasData, (), (const, override));
        MOCK_METHOD(bool, HasFramingError, (), (const, override));
        MOCK_METHOD(void, ReadFile, (std::string), (override));
        MOCK_METHOD(void, SaveFile, (std::string), (override));
        MOCK_METHOD(void, SetStepperPhases, (int), (override));
        MOCK_METHOD(bool, Mount, (const std::string&, std::string&), (override));
        MOCK_METHOD(void, Eject, (), (override));
        MOCK_METHOD(bool, IsMounted, (), (const, override));
        MOCK_METHOD(std::string, GetMountedPath, (), (const, override));
        MOCK_METHOD(bool, IsModified, (), (const, override));
        MOCK_METHOD(bool, Save, (std::string&), (override));
        MOCK_METHOD(bool, SaveAs, (const std::string&, std::string&), (override));
};
