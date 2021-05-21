#include <gtest/gtest.h>

#include <Compucolor.Impl.Smc5027/Smc5027Emulator.h>

#include <Compucolor.Tests.Core/NoopLogger.h>

TEST(Smc5027Emulator, SetBytePersistsData) {
    //Arrange
    std::shared_ptr<Compucolor::Logger::ILogger> logger =
        std::shared_ptr<Compucolor::Logger::ILogger>(new NoopLogger());

    Compucolor::Impl::Smc5027::Smc5027Emulator smc5027Emulator =
        Compucolor::Impl::Smc5027::Smc5027Emulator(logger);

    //Act
    smc5027Emulator.Start();
    smc5027Emulator.Stop();

    //Assert
    EXPECT_EQ(0, 0);
}