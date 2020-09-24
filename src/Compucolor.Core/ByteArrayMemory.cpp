#include "ByteArrayMemory.h"

ByteArrayMemory::ByteArrayMemory():
    _data(std::vector<uint8_t>(64 * 1024))
{
}

uint8_t ByteArrayMemory::GetByte(uint16_t address)
{
    return _data[address];
}

void ByteArrayMemory::SetByte(uint16_t address, uint8_t data)
{
    _data[address] = data;
}