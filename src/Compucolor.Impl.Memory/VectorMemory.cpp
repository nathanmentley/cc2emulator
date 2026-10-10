#include "VectorMemory.h"

Compucolor::Impl::Memory::VectorMemory::VectorMemory():
    _data{}
{
}

// 0x6000-0x6FFF is the fast-access alias of video RAM at 0x7000-0x7FFF.
static uint16_t MapAddress(const uint16_t address)
{
    return (address >= 0x6000 && address < 0x7000) ? address + 0x1000 : address;
}

uint8_t Compucolor::Impl::Memory::VectorMemory::GetByte(const uint16_t address)
{
    return _data[MapAddress(address)].load(std::memory_order_relaxed);
}

void Compucolor::Impl::Memory::VectorMemory::SetByte(const uint16_t address, const uint8_t data)
{
    _data[MapAddress(address)].store(data, std::memory_order_relaxed);
}

std::shared_ptr<Compucolor::Memory::IMemory> Compucolor::Impl::Memory::VectorMemory::Creator(
    Common::IPluginManager* manager
)
{
    return std::make_shared<VectorMemory>();
}