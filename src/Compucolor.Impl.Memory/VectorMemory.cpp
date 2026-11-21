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

void Compucolor::Impl::Memory::VectorMemory::Start() {
}

void Compucolor::Impl::Memory::VectorMemory::Stop() {
}

std::unique_ptr<boost::json::value> Compucolor::Impl::Memory::VectorMemory::SaveState()
{
    boost::json::array memory;

    for (const auto& byte : _data)
    {
        memory.push_back(byte.load(std::memory_order_relaxed));
    }

    return std::make_unique<boost::json::value>(std::move(memory));
}

void Compucolor::Impl::Memory::VectorMemory::LoadState(
    std::shared_ptr<boost::json::value> state
)
{
    if (!state || !state->is_array())
    {
        throw std::runtime_error("Invalid VectorMemory state");
    }

    const auto& memory = state->as_array();

    if (memory.size() != _data.size())
    {
        throw std::runtime_error("VectorMemory state has incorrect size");
    }

    for (size_t i = 0; i < _data.size(); ++i)
    {
        uint64_t value;

        if (memory[i].is_uint64())
        {
            value = memory[i].as_uint64();
        }
        else if (memory[i].is_int64() && memory[i].as_int64() >= 0)
        {
            value = static_cast<uint64_t>(memory[i].as_int64());
        }
        else
        {
            throw std::runtime_error("Invalid byte in VectorMemory state");
        }

        if (value > 255)
        {
            throw std::runtime_error("Invalid byte value in VectorMemory state");
        }

        _data[i].store(
            static_cast<uint8_t>(value),
            std::memory_order_relaxed
        );
    }
}

std::shared_ptr<Compucolor::Memory::IMemory> Compucolor::Impl::Memory::VectorMemory::Creator(
    Common::IPluginManager* manager
)
{
    return std::make_shared<VectorMemory>();
}