#include <gtest/gtest.h>

#include <Compucolor.Memory.Impl/VectorMemory.h>

TEST(VectorMemory, SetBytePersistsData) {
    //Arrange
    const uint8_t value = 0x12;
    const uint16_t address = 0x100;
    Compucolor::Memory::Impl::VectorMemory memory =
        Compucolor::Memory::Impl::VectorMemory();

    //Act
    memory.SetByte(address, value);

    //Assert
    EXPECT_EQ(value, memory.GetByte(address));
}

TEST(VectorMemory, GetByteFetchesPersistedData) {
    //Arrange
    const uint8_t value = 0x12;
    const uint16_t address = 0x100;
    Compucolor::Memory::Impl::VectorMemory memory =
        Compucolor::Memory::Impl::VectorMemory();
    memory.SetByte(address, value);

    //Act
    uint8_t result = memory.GetByte(address);

    //Assert
    EXPECT_EQ(value, result);
}