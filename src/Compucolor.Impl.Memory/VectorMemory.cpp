#include "VectorMemory.h"

Compucolor::Impl::Memory::VectorMemory::VectorMemory():
    _data{}
{
}

uint8_t Compucolor::Impl::Memory::VectorMemory::GetByte(const uint16_t address)
{
    return _data[address].load(std::memory_order_relaxed);
}

void Compucolor::Impl::Memory::VectorMemory::SetByte(const uint16_t address, const uint8_t data)
{
    _data[address].store(data, std::memory_order_relaxed);
}

std::shared_ptr<Compucolor::Memory::IMemory> Compucolor::Impl::Memory::VectorMemory::Creator(
    Common::IPluginManager* manager
)
{
    return std::make_shared<VectorMemory>();
}