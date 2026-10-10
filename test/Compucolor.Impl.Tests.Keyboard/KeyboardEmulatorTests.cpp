#include <memory>

#include <gtest/gtest.h>

#include <Compucolor/Logger/ILogger.h>

#include "KeyboardEmulator.h"

using Compucolor::Keyboard::CompucolorIIKey;
using Compucolor::Impl::Keyboard::KeyboardEmulator;

namespace
{
    class NullLogger: public Compucolor::Logger::ILogger {
        public:
            void Log(Compucolor::Logger::LogLevel, std::string, ...) override {}
            void LogTrace(std::string, ...) override {}
            void LogDebug(std::string, ...) override {}
            void LogInformation(std::string, ...) override {}
            void LogWarning(std::string, ...) override {}
            void LogError(std::string, ...) override {}
    };

    class KeyboardEmulatorTest: public ::testing::Test {
        protected:
            KeyboardEmulator keyboard{std::make_shared<NullLogger>()};

            void SetUp() override { keyboard.Reset(); }
    };
}

TEST_F(KeyboardEmulatorTest, NothingPressedReadsAllOnes) {
    //Assert
    for (uint8_t row = 0; row < 17; row++)
    {
        EXPECT_EQ(0xFF, keyboard.Read(row)) << "row " << static_cast<int>(row);
    }
}

TEST_F(KeyboardEmulatorTest, ColonAndSemiColonUseSeparateRows) {
    //Act
    keyboard.OnKeyDown(CompucolorIIKey::Colon);
    keyboard.OnKeyDown(CompucolorIIKey::SemiColon);

    //Assert
    EXPECT_EQ(0xFE, keyboard.Read(5));
    EXPECT_EQ(0xFE, keyboard.Read(4));
}

TEST_F(KeyboardEmulatorTest, PunctuationFollowsMatrixOrder) {
    //Act
    keyboard.OnKeyDown(CompucolorIIKey::Comma);
    keyboard.OnKeyDown(CompucolorIIKey::Dash);
    keyboard.OnKeyDown(CompucolorIIKey::Period);
    keyboard.OnKeyDown(CompucolorIIKey::Slash);

    //Assert
    EXPECT_EQ(0xFE, keyboard.Read(3));
    EXPECT_EQ(0xFE, keyboard.Read(2));
    EXPECT_EQ(0xFE, keyboard.Read(1));
    EXPECT_EQ(0xFE, keyboard.Read(0));
}

TEST_F(KeyboardEmulatorTest, FunctionAndCursorKeysAreMapped) {
    //Act
    keyboard.OnKeyDown(CompucolorIIKey::F5);
    keyboard.OnKeyDown(CompucolorIIKey::CurUp);
    keyboard.OnKeyDown(CompucolorIIKey::Space);

    //Assert
    EXPECT_EQ(0xF7, keyboard.Read(10));
    EXPECT_EQ(0xDF, keyboard.Read(3));
    EXPECT_EQ(0xBF, keyboard.Read(15));
}

TEST_F(KeyboardEmulatorTest, ReleasingAKeyKeepsOtherKeysInTheSameRowPressed) {
    //Arrange
    keyboard.OnKeyDown(CompucolorIIKey::Shift);
    keyboard.OnKeyDown(CompucolorIIKey::Ctrl);

    //Act
    keyboard.OnKeyUp(CompucolorIIKey::Shift);

    //Assert: only Ctrl (bit 4) stays down.
    EXPECT_EQ(0xEF, keyboard.Read(16));
}
