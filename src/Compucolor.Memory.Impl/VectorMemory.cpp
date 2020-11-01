#include <Compucolor.Memory.Impl/VectorMemory.h>

VectorMemory::VectorMemory():
    _data(std::vector<uint8_t>(64 * 1024))
{
}

uint8_t VectorMemory::GetByte(uint16_t address)
{
    return _data[address];
}

void VectorMemory::SetByte(uint16_t address, uint8_t data)
{
    _data[address] = data;
}