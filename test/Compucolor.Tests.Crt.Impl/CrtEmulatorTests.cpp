#include <gtest/gtest.h>

#include <Compucolor.Crt.Impl/CrtEmulator.h>

#include <Compucolor.Tests.Core/MockMemory.h>
//#include <Compucolor.Tests.Core/MockScheduler.h>
#include <Compucolor.Tests.Core/MockSmc5027Emulator.h>
#include <Compucolor.Tests.Core/NoopLogger.h>
/*
TEST(Smc5027Emulator, SetBytePersistsData) {
    //Arrange
    std::shared_ptr<ILogger> logger = std::shared_ptr<ILogger>(new NoopLogger());
    std::shared_ptr<IMemory> memory = std::shared_ptr<IMemory>(new MockMemory());
    std::shared_ptr<IScheduler> scheduler = std::shared_ptr<IScheduler>(new MockScheduler());
    std::shared_ptr<ISmc5027Emulator> smc5027Emulator = std::shared_ptr<ISmc5027Emulator>(new MockSmc5027Emulator());

    CrtEmulator crtEmulator = CrtEmulator(
        logger,
        scheduler,
        memory,
        smc5027Emulator
    );

    //Act
    crtEmulator.Start();
    crtEmulator.Stop();

    //Assert
    EXPECT_EQ(0, 0);
}
*/