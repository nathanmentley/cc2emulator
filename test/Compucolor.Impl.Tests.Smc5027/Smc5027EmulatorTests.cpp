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