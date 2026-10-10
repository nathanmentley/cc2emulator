#include <algorithm>
#include <array>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CompucolorEmulator.h"
#include "Roms.h"

#include <Compucolor.Tests.Core/FakeScheduler.h>
#include <Compucolor.Tests.Core/MockCrtEmulator.h>
#include <Compucolor.Tests.Core/MockFloppyEmulator.h>
#include <Compucolor.Tests.Core/MockIntel8080Emulator.h>
#include <Compucolor.Tests.Core/MockKeyboardEmulator.h>
#include <Compucolor.Tests.Core/MockMemory.h>
#include <Compucolor.Tests.Core/MockSmc5027Emulator.h>
#include <Compucolor.Tests.Core/MockTms5501Emulator.h>

using Compucolor::Impl::CompucolorII::CompucolorEmulator;
using Compucolor::Keyboard::CompucolorIIKey;
using Compucolor::RomVersion;

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

namespace
{
    constexpr uint16_t PowerUpFlag = 0x81AF + 8;

    class CompucolorEmulatorTest: public ::testing::Test {
        protected:
            std::array<uint8_t, 65536> ram{};
            uint64_t nowNanoseconds = 1'000'000'000;

            std::shared_ptr<NiceMock<MockCrtEmulator>> crt = std::make_shared<NiceMock<MockCrtEmulator>>();
            std::shared_ptr<NiceMock<MockFloppyEmulator>> floppyA = std::make_shared<NiceMock<MockFloppyEmulator>>();
            std::shared_ptr<NiceMock<MockFloppyEmulator>> floppyB = std::make_shared<NiceMock<MockFloppyEmulator>>();
            std::shared_ptr<NiceMock<MockIntel8080Emulator>> intel8080 = std::make_shared<NiceMock<MockIntel8080Emulator>>();
            std::shared_ptr<NiceMock<MockKeyboardEmulator>> keyboard = std::make_shared<NiceMock<MockKeyboardEmulator>>();
            std::shared_ptr<NiceMock<MockMemory>> memory = std::make_shared<NiceMock<MockMemory>>();
            std::shared_ptr<FakeScheduler> scheduler = std::make_shared<FakeScheduler>();
            std::shared_ptr<NiceMock<MockSmc5027Emulator>> smc5027 = std::make_shared<NiceMock<MockSmc5027Emulator>>();
            std::shared_ptr<NiceMock<MockTms5501Emulator>> tms5501 = std::make_shared<NiceMock<MockTms5501Emulator>>();

            std::unique_ptr<CompucolorEmulator> emulator;

            void SetUp() override
            {
                ON_CALL(*memory, SetByte(_, _)).WillByDefault([this](uint16_t address, uint8_t data) { ram[address] = data; });
                ON_CALL(*memory, GetByte(_)).WillByDefault([this](uint16_t address) { return ram[address]; });
                ON_CALL(*intel8080, Step()).WillByDefault(Return(4));

                emulator = std::make_unique<CompucolorEmulator>(
                    crt, floppyA, floppyB, intel8080, keyboard, memory, scheduler, smc5027, tms5501,
                    [this] { return nowNanoseconds; }
                );
            }
        };

    bool RomLoaded(const std::array<uint8_t, 65536>& ram, const uint8_t* rom, uint16_t length)
    {
        return std::equal(rom, rom + length, ram.begin());
    }
}

TEST_F(CompucolorEmulatorTest, GetFloppyReturnsDrivesByIndex) {
    //Assert
    EXPECT_EQ(floppyA, emulator->GetFloppy(0));
    EXPECT_EQ(floppyB, emulator->GetFloppy(1));
    EXPECT_EQ(nullptr, emulator->GetFloppy(2));
    EXPECT_EQ(nullptr, emulator->GetFloppy(-1));
}

TEST_F(CompucolorEmulatorTest, StartsUnpausedAtRealSpeed) {
    //Assert
    EXPECT_FALSE(emulator->IsPaused());
    EXPECT_TRUE(emulator->IsSpeedRegulated());
}

TEST_F(CompucolorEmulatorTest, PauseAndResumeToggleThePausedState) {
    //Act
    emulator->Pause();
    const bool pausedAfterPause = emulator->IsPaused();
    emulator->Resume();

    //Assert
    EXPECT_TRUE(pausedAfterPause);
    EXPECT_FALSE(emulator->IsPaused());
}

TEST_F(CompucolorEmulatorTest, StartStartsDevicesAndLoadsTheSystemRom) {
    //Arrange
    EXPECT_CALL(*crt, Start()).Times(1);
    EXPECT_CALL(*keyboard, Start()).Times(1);
    EXPECT_CALL(*smc5027, Start()).Times(1);
    EXPECT_CALL(*tms5501, Start()).Times(1);
    EXPECT_CALL(*intel8080, Start()).Times(1);

    //Act
    emulator->Start();

    //Assert
    EXPECT_EQ(1'000'000u, scheduler->interval);
    EXPECT_TRUE(RomLoaded(ram, Compucolor::Impl::CompucolorII::get_system_rom_8_79(), Compucolor::Impl::CompucolorII::get_system_rom_8_79_length()));
}

TEST_F(CompucolorEmulatorTest, StopStopsDevices) {
    //Arrange
    emulator->Start();
    EXPECT_CALL(*crt, Stop()).Times(1);
    EXPECT_CALL(*keyboard, Stop()).Times(1);
    EXPECT_CALL(*smc5027, Stop()).Times(1);
    EXPECT_CALL(*tms5501, Stop()).Times(1);
    EXPECT_CALL(*intel8080, Stop()).Times(1);

    //Act
    emulator->Stop();
}

TEST_F(CompucolorEmulatorTest, RealSpeedRunsTheCyclesThatElapsedTimeAllows) {
    //Arrange: a 4 cycle step takes 2008 ns, so 6024 ns allows exactly 3 steps.
    emulator->Start();
    EXPECT_CALL(*intel8080, Step()).Times(3).WillRepeatedly(Return(4));
    nowNanoseconds += 3 * 4 * 502;

    //Act
    scheduler->Tick();
}

TEST_F(CompucolorEmulatorTest, RealSpeedDoesNothingWhenNoTimeHasPassed) {
    //Arrange
    emulator->Start();
    EXPECT_CALL(*intel8080, Step()).Times(0);

    //Act
    scheduler->Tick();
}

TEST_F(CompucolorEmulatorTest, RealSpeedCarriesLeftoverTimeToTheNextTick) {
    //Arrange: 3000 ns is one 2008 ns step plus 992 ns carried over.
    int steps = 0;
    ON_CALL(*intel8080, Step()).WillByDefault([&steps] { steps++; return 4; });
    emulator->Start();

    //Act
    nowNanoseconds += 3000;
    scheduler->Tick();
    const int afterFirstTick = steps;
    nowNanoseconds += 3000;
    scheduler->Tick();

    //Assert: 6000 ns in total pays for three steps.
    EXPECT_EQ(2, afterFirstTick);
    EXPECT_EQ(3, steps);
}

TEST_F(CompucolorEmulatorTest, RealSpeedCapsCatchUpAfterALongStall) {
    //Arrange
    int steps = 0;
    ON_CALL(*intel8080, Step()).WillByDefault([&steps] { steps++; return 4; });
    emulator->Start();

    //Act: ten seconds pass between ticks
    nowNanoseconds += 10'000'000'000;
    scheduler->Tick();

    //Assert: at most 50 ms of emulated time is replayed.
    EXPECT_LE(steps, 50'000'000 / 2008 + 1);
    EXPECT_GT(steps, 20'000);
}

TEST_F(CompucolorEmulatorTest, StepsThatReportNoCyclesStillAdvanceTime) {
    //Arrange
    int steps = 0;
    ON_CALL(*intel8080, Step()).WillByDefault([&steps] { steps++; return 0; });
    emulator->Start();

    //Act
    nowNanoseconds += 5 * 502;
    scheduler->Tick();

    //Assert
    EXPECT_EQ(5, steps);
}

TEST_F(CompucolorEmulatorTest, TimeSpentPausedIsNotReplayedOnResume) {
    //Arrange
    int steps = 0;
    ON_CALL(*intel8080, Step()).WillByDefault([&steps] { steps++; return 4; });
    emulator->Start();
    emulator->Pause();

    //Act
    nowNanoseconds += 5'000'000'000;
    scheduler->Tick();
    emulator->Resume();
    nowNanoseconds += 2 * 4 * 502;
    scheduler->Tick();

    //Assert
    EXPECT_EQ(2, steps);
}

TEST_F(CompucolorEmulatorTest, UnregulatedSpeedStepsTheCpuInABurst) {
    //Arrange
    emulator->Start();
    emulator->SetSpeedRegulated(false);
    EXPECT_CALL(*intel8080, Step()).Times(4096).WillRepeatedly(Return(4));

    //Act
    scheduler->Tick();

    //Assert
    EXPECT_FALSE(emulator->IsSpeedRegulated());
}

TEST_F(CompucolorEmulatorTest, PausedEmulatorDoesNotStepTheCpu) {
    //Arrange
    emulator->Start();
    emulator->Pause();
    EXPECT_CALL(*intel8080, Step()).Times(0);
    nowNanoseconds += 10'000'000;

    //Act
    scheduler->Tick();
}

TEST_F(CompucolorEmulatorTest, WarmResetResetsDevicesAndKeepsMemory) {
    //Arrange
    emulator->Start();
    ram[PowerUpFlag] = 0x97;
    EXPECT_CALL(*intel8080, Reset()).Times(1);
    EXPECT_CALL(*tms5501, Reset()).Times(1);
    EXPECT_CALL(*smc5027, Reset()).Times(1);
    EXPECT_CALL(*keyboard, Reset()).Times(1);

    //Act
    emulator->Reset(false);
    scheduler->Tick();

    //Assert
    EXPECT_EQ(0x97, ram[PowerUpFlag]);
}

TEST_F(CompucolorEmulatorTest, HardResetClearsThePowerUpFlagAndReloadsTheRom) {
    //Arrange
    emulator->Start();
    ram[PowerUpFlag] = 0x97;
    ram[0] = 0xAA;

    //Act
    emulator->Reset(true);
    scheduler->Tick();

    //Assert
    EXPECT_EQ(0x00, ram[PowerUpFlag]);
    EXPECT_TRUE(RomLoaded(ram, Compucolor::Impl::CompucolorII::get_system_rom_8_79(), Compucolor::Impl::CompucolorII::get_system_rom_8_79_length()));
}

TEST_F(CompucolorEmulatorTest, ResetIsAppliedEvenWhilePaused) {
    //Arrange
    emulator->Start();
    emulator->Pause();
    EXPECT_CALL(*intel8080, Reset()).Times(1);
    EXPECT_CALL(*intel8080, Step()).Times(0);

    //Act
    emulator->Reset(false);
    scheduler->Tick();
}

TEST_F(CompucolorEmulatorTest, SelectingAnotherRomHardResetsIntoThatRom) {
    //Arrange
    using namespace Compucolor::Impl::CompucolorII;
    const uint16_t length = std::min(get_system_rom_6_78_length(), get_system_rom_8_79_length());
    ASSERT_FALSE(std::equal(get_system_rom_6_78(), get_system_rom_6_78() + length, get_system_rom_8_79()));
    emulator->Start();

    //Act
    emulator->SetRomVersion(RomVersion::V6_78);
    scheduler->Tick();

    //Assert
    EXPECT_TRUE(RomLoaded(ram, get_system_rom_6_78(), get_system_rom_6_78_length()));
    EXPECT_EQ(0x00, ram[PowerUpFlag]);
}

TEST_F(CompucolorEmulatorTest, CharsetChangesAreForwardedToTheCrt) {
    //Arrange
    EXPECT_CALL(*crt, SetLowercaseCharset(true)).Times(1);

    //Act
    emulator->SetLowercaseCharset(true);
}

TEST_F(CompucolorEmulatorTest, KeyEventsAreForwardedToTheKeyboard) {
    //Arrange
    EXPECT_CALL(*keyboard, OnKeyDown(CompucolorIIKey::A)).Times(1);
    EXPECT_CALL(*keyboard, OnKeyUp(CompucolorIIKey::A)).Times(1);

    //Act
    emulator->OnKeyDown(CompucolorIIKey::A);
    emulator->OnKeyUp(CompucolorIIKey::A);
}

TEST_F(CompucolorEmulatorTest, StartConfiguresFourPhaseSteppersForTheDefaultRom) {
    //Arrange
    EXPECT_CALL(*floppyA, SetStepperPhases(4)).Times(1);
    EXPECT_CALL(*floppyB, SetStepperPhases(4)).Times(1);

    //Act
    emulator->Start();
}

TEST_F(CompucolorEmulatorTest, OldRomSwitchesBothDrivesToThreePhaseSteppers) {
    //Arrange
    emulator->Start();
    EXPECT_CALL(*floppyA, SetStepperPhases(3)).Times(1);
    EXPECT_CALL(*floppyB, SetStepperPhases(3)).Times(1);

    //Act
    emulator->SetRomVersion(RomVersion::V6_78);
    scheduler->Tick();
}

TEST_F(CompucolorEmulatorTest, ReturningToTheNewRomRestoresFourPhaseSteppers) {
    //Arrange
    emulator->Start();
    emulator->SetRomVersion(RomVersion::V6_78);
    scheduler->Tick();
    EXPECT_CALL(*floppyA, SetStepperPhases(4)).Times(1);
    EXPECT_CALL(*floppyB, SetStepperPhases(4)).Times(1);

    //Act
    emulator->SetRomVersion(RomVersion::V8_79);
    scheduler->Tick();
}
