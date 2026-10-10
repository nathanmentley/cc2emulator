#include <gtest/gtest.h>

#include "Smc5027Emulator.h"

#include <Compucolor.Tests.Core/NoopLogger.h>

TEST(Smc5027Emulator, DisplayRowTracksScrollRegister)
{
    std::shared_ptr<Compucolor::Logger::ILogger> logger =
        std::shared_ptr<Compucolor::Logger::ILogger>(new NoopLogger());
    Compucolor::Impl::Smc5027::Smc5027Emulator smc5027Emulator =
        Compucolor::Impl::Smc5027::Smc5027Emulator(logger);

    smc5027Emulator.Write(0x06, 0x1F);
    EXPECT_EQ(0, smc5027Emulator.FirstDisplayRow());

    smc5027Emulator.Write(0x0B, 0x00);
    EXPECT_EQ(1, smc5027Emulator.FirstDisplayRow());
}

namespace
{
    Compucolor::Impl::Smc5027::Smc5027Emulator MakeSmc5027()
    {
        return Compucolor::Impl::Smc5027::Smc5027Emulator(
            std::shared_ptr<Compucolor::Logger::ILogger>(new NoopLogger())
        );
    }
}

TEST(Smc5027Emulator, ScrollWrapsAroundTheThirtyTwoRows)
{
    //Arrange
    auto smc5027 = MakeSmc5027();
    smc5027.Write(0x06, 0x1F);

    //Act
    smc5027.Write(0x0B, 0x00);
    smc5027.Write(0x0B, 0x00);

    //Assert
    EXPECT_EQ(2, smc5027.FirstDisplayRow());
}

TEST(Smc5027Emulator, CursorRegistersAreLoadedThroughTheirPorts)
{
    //Arrange
    auto smc5027 = MakeSmc5027();

    //Act
    smc5027.Write(0x0C, 12);
    smc5027.Write(0x0D, 7);

    //Assert
    EXPECT_EQ(12, smc5027.GetCursorX());
    EXPECT_EQ(7, smc5027.GetCursorY());
}

TEST(Smc5027Emulator, CursorYKeepsOnlySixBits)
{
    //Arrange
    auto smc5027 = MakeSmc5027();

    //Act
    smc5027.Write(0x0D, 0xFF);

    //Assert
    EXPECT_EQ(0x3F, smc5027.GetCursorY());
}

TEST(Smc5027Emulator, PortAddressIsMirroredAcrossTheChipSelectRange)
{
    //Arrange
    auto smc5027 = MakeSmc5027();

    //Act: the bus only forwards the low nibble, but higher bits must not matter.
    smc5027.Write(0x7C, 33);

    //Assert
    EXPECT_EQ(33, smc5027.GetCursorX());
}

TEST(Smc5027Emulator, ResetClearsCursorAndScroll)
{
    //Arrange
    auto smc5027 = MakeSmc5027();
    smc5027.Write(0x0C, 5);
    smc5027.Write(0x0D, 6);
    smc5027.Write(0x06, 0x10);

    //Act
    smc5027.Reset();

    //Assert
    EXPECT_EQ(0, smc5027.GetCursorX());
    EXPECT_EQ(0, smc5027.GetCursorY());
    EXPECT_EQ(1, smc5027.FirstDisplayRow());
}
