#include "Intel8080EmulatorContext.h"

Intel8080EmulatorContext::Intel8080EmulatorContext():
    _bus({}),
    _memory({})
{
}

void Intel8080EmulatorContext::SetMemory(std::shared_ptr<IMemory> memory)
{
    _memory = memory;
}

void Intel8080EmulatorContext::SetBus(std::shared_ptr<IIntel8080Bus> intel8080Bus)
{
    _bus = intel8080Bus;
}

std::optional<std::shared_ptr<IMemory>> Intel8080EmulatorContext::GetMemory()
{
    return _memory;
}

std::optional<std::shared_ptr<IIntel8080Bus>> Intel8080EmulatorContext::GetBus()
{
    return _bus;
}