#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Tms5501Emulator.h"

#include <Compucolor.Tests.Core/MockAudioOutput.h>
#include <Compucolor.Tests.Core/MockFloppyEmulator.h>
#include <Compucolor.Tests.Core/MockIntel8080Emulator.h>
#include <Compucolor.Tests.Core/MockKeyboardEmulator.h>
#include <Compucolor.Tests.Core/NoopLogger.h>

using Compucolor::Impl::Tms5501::Tms5501Emulator;

using ::testing::NiceMock;
using ::testing::_;

namespace
{
    constexpr uint8_t DiscreteCommandPort = 0x04;
    constexpr uint8_t OutputPort = 0x07;
    constexpr uint8_t BreakBit = 0x02;

    class Tms5501EmulatorTest: public ::testing::Test {
        protected:
            std::shared_ptr<NiceMock<MockAudioOutput>> audio = std::make_shared<NiceMock<MockAudioOutput>>();
            std::shared_ptr<NiceMock<MockFloppyEmulator>> floppy1 = std::make_shared<NiceMock<MockFloppyEmulator>>();
            std::shared_ptr<NiceMock<MockFloppyEmulator>> floppy2 = std::make_shared<NiceMock<MockFloppyEmulator>>();
            std::unique_ptr<Tms5501Emulator> tms5501;

            void SetUp() override
            {
                tms5501 = Make(audio);
            }

            std::unique_ptr<Tms5501Emulator> Make(std::shared_ptr<Compucolor::Common::IAudioOutput> output)
            {
                return std::make_unique<Tms5501Emulator>(
                    std::make_shared<NoopLogger>(),
                    std::make_shared<NiceMock<MockIntel8080Emulator>>(),
                    std::make_shared<NiceMock<MockKeyboardEmulator>>(),
                    floppy1,
                    floppy2,
                    std::move(output)
                );
            }
        };
}

TEST_F(Tms5501EmulatorTest, BreakBitDrivesTheAudioLine) {
    //Arrange
    ::testing::InSequence sequence;
    EXPECT_CALL(*audio, SetLevel(true, 1000)).Times(1);
    EXPECT_CALL(*audio, SetLevel(false, 3000)).Times(1);

    //Act
    tms5501->SetEmulatedTime(1000);
    tms5501->Write(DiscreteCommandPort, BreakBit);
    tms5501->SetEmulatedTime(3000);
    tms5501->Write(DiscreteCommandPort, 0x00);
}

TEST_F(Tms5501EmulatorTest, UnchangedBreakBitSendsNoEdge) {
    //Arrange
    EXPECT_CALL(*audio, SetLevel(_, _)).Times(1);

    //Act
    tms5501->Write(DiscreteCommandPort, BreakBit);
    tms5501->Write(DiscreteCommandPort, BreakBit | 0x08);
    tms5501->Write(DiscreteCommandPort, BreakBit);
}

TEST_F(Tms5501EmulatorTest, ClearBreakBitOnAnIdleLineSendsNothing) {
    //Arrange
    EXPECT_CALL(*audio, SetLevel(_, _)).Times(0);

    //Act
    tms5501->Write(DiscreteCommandPort, 0x00);
}

TEST_F(Tms5501EmulatorTest, OtherDevicesSelectedMeansNoAudio) {
    //Arrange: output bits 5:4 = 1 selects floppy drive A instead of the serial port
    tms5501->Write(OutputPort, 0x10);
    EXPECT_CALL(*audio, SetLevel(_, _)).Times(0);

    //Act
    tms5501->Write(DiscreteCommandPort, BreakBit);
}

TEST_F(Tms5501EmulatorTest, ResetReleasesTheAudioLine) {
    //Arrange
    tms5501->SetEmulatedTime(500);
    tms5501->Write(DiscreteCommandPort, BreakBit);
    EXPECT_CALL(*audio, SetLevel(false, 500)).Times(1);

    //Act
    tms5501->Reset();
}

TEST_F(Tms5501EmulatorTest, ResetOnAQuietLineSendsNothing) {
    //Arrange
    EXPECT_CALL(*audio, SetLevel(_, _)).Times(0);

    //Act
    tms5501->Reset();
}

TEST_F(Tms5501EmulatorTest, FloppyStepsReportEmulatedTimeToAudio) {
    //Arrange
    EXPECT_CALL(*audio, Advance(123456)).Times(1);
    tms5501->SetEmulatedTime(123456);

    //Act
    tms5501->StepFloppy(16'000);
}

TEST_F(Tms5501EmulatorTest, WorksWithoutAnAudioOutput) {
    //Arrange
    auto silent = Make(nullptr);

    //Act
    silent->Write(DiscreteCommandPort, BreakBit);
    silent->StepFloppy(16'000);
    silent->Reset();

    //Assert: reaching here without a crash is the check
    SUCCEED();
}
