#include <gtest/gtest.h>

#include "VectorMemory.h"

TEST(VectorMemory, SetBytePersistsData) {
    //Arrange
    const uint8_t value = 0x12;
    const uint16_t address = 0x100;
    Compucolor::Impl::Memory::VectorMemory memory =
        Compucolor::Impl::Memory::VectorMemory();

    //Act
    memory.SetByte(address, value);

    //Assert
    EXPECT_EQ(value, memory.GetByte(address));
}

TEST(VectorMemory, GetByteFetchesPersistedData) {
    //Arrange
    const uint8_t value = 0x12;
    const uint16_t address = 0x100;
    Compucolor::Impl::Memory::VectorMemory memory =
        Compucolor::Impl::Memory::VectorMemory();
    memory.SetByte(address, value);

    //Act
    uint8_t result = memory.GetByte(address);

    //Assert
    EXPECT_EQ(value, result);
}

TEST(VectorMemory, FastVideoRamWritesAppearInSlowVideoRam) {
    //Arrange
    Compucolor::Impl::Memory::VectorMemory memory;

    //Act
    memory.SetByte(0x6123, 0x5A);

    //Assert
    EXPECT_EQ(0x5A, memory.GetByte(0x7123));
}

TEST(VectorMemory, SlowVideoRamWritesAppearInFastVideoRam) {
    //Arrange
    Compucolor::Impl::Memory::VectorMemory memory;

    //Act
    memory.SetByte(0x7FFF, 0xC3);

    //Assert
    EXPECT_EQ(0xC3, memory.GetByte(0x6FFF));
}

TEST(VectorMemory, AddressesOutsideTheVideoAliasAreIndependent) {
    //Arrange
    Compucolor::Impl::Memory::VectorMemory memory;

    //Act
    memory.SetByte(0x5FFF, 0x11);
    memory.SetByte(0x8000, 0x22);

    //Assert
    EXPECT_EQ(0x11, memory.GetByte(0x5FFF));
    EXPECT_EQ(0x00, memory.GetByte(0x6FFF));
    EXPECT_EQ(0x22, memory.GetByte(0x8000));
    EXPECT_EQ(0x00, memory.GetByte(0x7000));
}
