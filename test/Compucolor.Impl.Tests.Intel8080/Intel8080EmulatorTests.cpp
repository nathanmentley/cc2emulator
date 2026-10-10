#include <array>
#include <memory>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Intel8080Emulator.h"

#include <Compucolor.Tests.Core/MockMemory.h>

using Compucolor::Impl::Intel8080::Intel8080Emulator;

using ::testing::NiceMock;
using ::testing::_;

namespace
{
    class Intel8080EmulatorTest: public ::testing::Test {
        protected:
            std::array<uint8_t, 65536> ram{};
            std::shared_ptr<NiceMock<MockMemory>> memory = std::make_shared<NiceMock<MockMemory>>();
            std::unique_ptr<Intel8080Emulator> cpu;

            void SetUp() override
            {
                ON_CALL(*memory, GetByte(_)).WillByDefault([this](uint16_t address) { return ram[address]; });
                ON_CALL(*memory, SetByte(_, _)).WillByDefault([this](uint16_t address, uint8_t data) { ram[address] = data; });

                cpu = std::make_unique<Intel8080Emulator>(memory);
            }

            // Loads a program at 0x0000 directly, the way the ROM is loaded, then runs it.
            void Run(const std::vector<uint8_t>& program, int steps)
            {
                std::copy(program.begin(), program.end(), ram.begin());

                cpu->Start();

                for (int step = 0; step < steps; step++)
                {
                    cpu->Step();
                }
            }

            static std::vector<uint8_t> StoreAccumulator(uint16_t address)
            {
                return {0x32, static_cast<uint8_t>(address & 0xFF), static_cast<uint8_t>(address >> 8)};
            }
    };
}

TEST_F(Intel8080EmulatorTest, WritesToTheRomAreIgnored) {
    //Arrange: MVI A,0x55 then STA to the first and last ROM addresses
    std::vector<uint8_t> program = {0x3E, 0x55};
    for (const uint16_t address : {0x0100, 0x3FFF, 0x0000})
    {
        const auto store = StoreAccumulator(address);
        program.insert(program.end(), store.begin(), store.end());
    }

    //Act
    Run(program, 4);

    //Assert
    EXPECT_EQ(0x00, ram[0x0100]);
    EXPECT_EQ(0x00, ram[0x3FFF]);
    EXPECT_EQ(0x3E, ram[0x0000]);
}

TEST_F(Intel8080EmulatorTest, WritesAboveTheRomAreStored) {
    //Arrange
    std::vector<uint8_t> program = {0x3E, 0x55};
    for (const uint16_t address : {0x4000, 0x7123, 0x8000, 0xFFFF})
    {
        const auto store = StoreAccumulator(address);
        program.insert(program.end(), store.begin(), store.end());
    }

    //Act
    Run(program, 5);

    //Assert
    EXPECT_EQ(0x55, ram[0x4000]);
    EXPECT_EQ(0x55, ram[0x7123]);
    EXPECT_EQ(0x55, ram[0x8000]);
    EXPECT_EQ(0x55, ram[0xFFFF]);
}

TEST_F(Intel8080EmulatorTest, RomWritesNeverReachTheMemoryPlugin) {
    //Arrange
    EXPECT_CALL(*memory, SetByte(_, _)).Times(0);
    std::vector<uint8_t> program = {0x3E, 0x55};
    const auto store = StoreAccumulator(0x2000);
    program.insert(program.end(), store.begin(), store.end());

    //Act
    Run(program, 2);
}

TEST_F(Intel8080EmulatorTest, FetchesInstructionsFromTheRom) {
    //Arrange: the program itself lives in the protected ROM area
    std::vector<uint8_t> program = {0x3E, 0x7E};
    const auto store = StoreAccumulator(0x9000);
    program.insert(program.end(), store.begin(), store.end());

    //Act
    Run(program, 2);

    //Assert
    EXPECT_EQ(0x7E, ram[0x9000]);
}
