#include "Intel8080EmulatorContext.h"

Compucolor::Intel8080::Impl::Intel8080EmulatorContext::Intel8080EmulatorContext():
    _bus({}),
    _memory({})
{
}

void Compucolor::Intel8080::Impl::Intel8080EmulatorContext::SetMemory(std::shared_ptr<Compucolor::Memory::IMemory> memory)
{
    _memory = memory;
}

void Compucolor::Intel8080::Impl::Intel8080EmulatorContext::SetBus(std::shared_ptr<IIntel8080Bus> intel8080Bus)
{
    _bus = intel8080Bus;
}

std::optional<std::shared_ptr<Compucolor::Memory::IMemory>> Compucolor::Intel8080::Impl::Intel8080EmulatorContext::GetMemory()
{
    return _memory;
}

std::optional<std::shared_ptr<Compucolor::Intel8080::IIntel8080Bus>> Compucolor::Intel8080::Impl::Intel8080EmulatorContext::GetBus()
{
    return _bus;
}