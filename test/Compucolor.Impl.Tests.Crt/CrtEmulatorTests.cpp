#include <array>
#include <cstring>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CrtEmulator.h"
#include "FontRom.h"

#include <Compucolor.Tests.Core/FakeDisplay.h>
#include <Compucolor.Tests.Core/FakeScheduler.h>
#include <Compucolor.Tests.Core/MockMemory.h>
#include <Compucolor.Tests.Core/MockSmc5027Emulator.h>
#include <Compucolor.Tests.Core/MockTms5501Emulator.h>
#include <Compucolor.Tests.Core/NoopLogger.h>

using Compucolor::Common::Color;
using Compucolor::Impl::Crt::CrtEmulator;

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

namespace
{
    constexpr uint16_t VideoRam = 0x7000;
    constexpr uint8_t White = 0x07;

    class CrtEmulatorTest: public ::testing::Test {
        protected:
            std::array<uint8_t, 65536> ram{};
            uint8_t firstRow = 0;
            uint8_t cursorX = 200;
            uint8_t cursorY = 200;

            std::shared_ptr<FakeDisplay> display = std::make_shared<FakeDisplay>();
            std::shared_ptr<FakeScheduler> scheduler = std::make_shared<FakeScheduler>(true);
            std::shared_ptr<NiceMock<MockMemory>> memory = std::make_shared<NiceMock<MockMemory>>();
            std::shared_ptr<NiceMock<MockSmc5027Emulator>> smc5027 = std::make_shared<NiceMock<MockSmc5027Emulator>>();
            std::shared_ptr<NiceMock<MockTms5501Emulator>> tms5501 = std::make_shared<NiceMock<MockTms5501Emulator>>();

            void SetUp() override
            {
                ON_CALL(*memory, GetByte(_)).WillByDefault([this](uint16_t address) { return ram[address]; });
                ON_CALL(*smc5027, FirstDisplayRow()).WillByDefault([this] { return firstRow; });
                // A cursor far off screen is not drawn, so it does not disturb the characters.
                ON_CALL(*smc5027, GetCursorX()).WillByDefault([this] { return cursorX; });
                ON_CALL(*smc5027, GetCursorY()).WillByDefault([this] { return cursorY; });
            }

            void Place(int row, int column, uint8_t character, uint8_t attribute)
            {
                const uint16_t address = VideoRam + row * 128 + column * 2;
                ram[address] = character;
                ram[address + 1] = attribute;
            }

            // The first refresh runs as soon as the CRT starts.
            void Render(bool lowercase = false)
            {
                CrtEmulator crt(std::make_shared<NoopLogger>(), display, scheduler, memory, smc5027, tms5501);
                crt.SetLowercaseCharset(lowercase);
                crt.Start();
                crt.Stop();
            }

            [[nodiscard]] std::optional<Color> Pixel(int cellColumn, int cellRow, int x, int y) const
            {
                return display->PixelAt(cellColumn * 6 + x, cellRow * 8 + y);
            }
    };
}

TEST_F(CrtEmulatorTest, TextCharacterDrawsFontPixelsInForegroundAndBackground) {
    //Arrange: red on blue
    const uint8_t attribute = 0x01 | (0x04 << 3);
    Place(0, 0, 'A', attribute);

    //Act
    Render();

    //Assert
    const uint8_t* rom = Compucolor::Impl::Crt::get_uf6_rom();
    for (int row = 0; row < 8; row++)
    {
        for (int column = 0; column < 6; column++)
        {
            const bool on = (rom['A' * 8 + row] >> (7 - column)) & 1;
            EXPECT_EQ(on ? Color::Red : Color::Blue, Pixel(0, 0, column, row)) << column << "," << row;
        }
    }
}

TEST_F(CrtEmulatorTest, CharacterCellsAreSixByEightPixels) {
    //Arrange: a plot block placed in the third column of the fourth row.
    Place(3, 2, 0x01, 0x80 | White);

    //Act
    Render();

    //Assert: plot block 0 covers the top-left 3x2 pixels of the cell at x=12, y=24.
    EXPECT_EQ(Color::White, display->PixelAt(12, 24));
    EXPECT_EQ(Color::White, display->PixelAt(14, 25));
    EXPECT_EQ(Color::Black, display->PixelAt(15, 24));
}

TEST_F(CrtEmulatorTest, PlotModeDrawsTwoByThreeBlockPatterns) {
    //Arrange: bit 0 is the top-left block, bit 7 the bottom-right block.
    Place(0, 0, 0x01, 0x80 | White);
    Place(0, 1, 0x80, 0x80 | White);

    //Act
    Render();

    //Assert
    EXPECT_EQ(Color::White, Pixel(0, 0, 0, 0));
    EXPECT_EQ(Color::White, Pixel(0, 0, 2, 1));
    EXPECT_EQ(Color::Black, Pixel(0, 0, 3, 0));
    EXPECT_EQ(Color::Black, Pixel(0, 0, 0, 2));

    EXPECT_EQ(Color::White, Pixel(1, 0, 3, 6));
    EXPECT_EQ(Color::White, Pixel(1, 0, 5, 7));
    EXPECT_EQ(Color::Black, Pixel(1, 0, 0, 6));
}

TEST_F(CrtEmulatorTest, BlinkAttributeHidesForegroundWhileBlinkIsOn) {
    //Arrange: bit 6 is blink; the blink phase starts on.
    Place(0, 0, 'A', 0x40 | White);

    //Act
    Render();

    //Assert
    for (int row = 0; row < 8; row++)
    {
        for (int column = 0; column < 6; column++)
        {
            EXPECT_EQ(Color::Black, Pixel(0, 0, column, row)) << column << "," << row;
        }
    }
}

TEST_F(CrtEmulatorTest, PlotBitIsNotTreatedAsBlink) {
    //Arrange
    Place(0, 0, 0x01, 0x80 | White);

    //Act
    Render();

    //Assert
    EXPECT_EQ(Color::White, Pixel(0, 0, 0, 0));
}

TEST_F(CrtEmulatorTest, HardwareScrollShiftsRowsUpAndWrapsTheTopRowToTheBottom) {
    //Arrange: memory row 1 becomes the first displayed row.
    firstRow = 1;
    Place(1, 0, 0x01, 0x80 | White);
    Place(0, 0, 0x01, 0x80 | 0x02);

    //Act
    Render();

    //Assert
    EXPECT_EQ(Color::White, Pixel(0, 0, 0, 0));
    EXPECT_EQ(Color::Green, Pixel(0, 31, 0, 0));
}

TEST_F(CrtEmulatorTest, DrawsTheCursorAsASolidCellAndAccountsForScroll) {
    //Arrange: cursor at memory row 2 with a one row scroll lands on screen row 1.
    firstRow = 1;
    cursorX = 4;
    cursorY = 2;

    //Act
    Render();

    //Assert
    for (int row = 0; row < 8; row++)
    {
        for (int column = 0; column < 6; column++)
        {
            EXPECT_EQ(Color::White, Pixel(4, 1, column, row)) << column << "," << row;
        }
    }
}

TEST_F(CrtEmulatorTest, LowercaseCharsetSelectsTheAlternateFont) {
    //Arrange
    const uint8_t character = 'a';
    const uint8_t* standard = Compucolor::Impl::Crt::get_uf6_rom();
    const uint8_t* lowercase = Compucolor::Impl::Crt::get_uf6_rom_lowercase();
    ASSERT_NE(0, std::memcmp(standard + character * 8, lowercase + character * 8, 8));
    Place(0, 0, character, White);

    //Act
    Render(true);

    //Assert
    for (int row = 0; row < 8; row++)
    {
        for (int column = 0; column < 6; column++)
        {
            const bool on = (lowercase[character * 8 + row] >> (7 - column)) & 1;
            EXPECT_EQ(on ? Color::White : Color::Black, Pixel(0, 0, column, row)) << column << "," << row;
        }
    }
}

TEST_F(CrtEmulatorTest, RepaintsOncePerRefresh) {
    //Act
    Render();

    //Assert
    EXPECT_EQ(1, display->repaints);
}
