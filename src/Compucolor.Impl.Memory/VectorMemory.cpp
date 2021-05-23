#include "VectorMemory.h"

Compucolor::Impl::Memory::VectorMemory::VectorMemory():
    _data(std::vector<uint8_t>(64 * 1024))
{
}

uint8_t Compucolor::Impl::Memory::VectorMemory::GetByte(uint16_t address)
{
    return _data[address];
}

void Compucolor::Impl::Memory::VectorMemory::SetByte(uint16_t address, uint8_t data)
{
    _data[address] = data;
}

std::shared_ptr<Compucolor::Memory::IMemory> Compucolor::Impl::Memory::VectorMemory::Creator(
    Compucolor::Common::PluginLoader* pluginLoader
)
{
    return std::shared_ptr<Compucolor::Memory::IMemory>(
        new Compucolor::Impl::Memory::VectorMemory()
    );
}