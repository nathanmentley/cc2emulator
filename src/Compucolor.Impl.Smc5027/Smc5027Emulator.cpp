#include "Smc5027Emulator.h"

#include <utility>

using std::make_shared;
using std::shared_ptr;

using Compucolor::Common::IPluginManager;

using Compucolor::Logger::ILogger;
using Compucolor::Smc5027::ISmc5027Emulator;

using Compucolor::Impl::Smc5027::Smc5027Emulator;

Smc5027Emulator::Smc5027Emulator(shared_ptr<ILogger> logger):
    _logger(std::move(logger))
{
    Smc5027Emulator::Reset();
}

void Smc5027Emulator::Start()
{
    _logger->LogTrace("Starting %s", "Smc5027Emulator");

    Reset();
}

void Smc5027Emulator::Stop()
{
    _logger->LogTrace("Stopping %s", "Smc5027Emulator");

    Reset();
}

uint8_t Smc5027Emulator::Read(uint8_t port)
{
    uint8_t data = 0x00;

    switch (ConvertPort(port))
    {
        case 0x08:
            data = _registers[0x08].load(std::memory_order_relaxed);
            break;

        case 0x09:
            data = _registers[0x09].load(std::memory_order_relaxed);
            break;

        default:
            _logger->LogWarning("Unimplemented port read from 0x%02x in %s", port, "Smc5027Emulator");
            break;
    }

    _logger->LogTrace(
        "Read data: 0x%02x from port: 0x%02x in %s",
        data,
        port,
        "Smc5027Emulator"
    );

    return data;
}

void Smc5027Emulator::Write(uint8_t port, uint8_t data)
{
    _logger->LogTrace("Writing data: 0x%02x to port: 0x%02x in %s", data, port, "Smc5027Emulator");

    switch (ConvertPort(port))
    {
        case 0x06:
            _registers[0x06].store(data, std::memory_order_relaxed);
            break;

        case 0x0B:
            _registers[0x06].store(
                (_registers[0x06].load(std::memory_order_relaxed) + 1) % ROW_COUNT,
                std::memory_order_relaxed
            );
            break;

        case 0x0C:
            _registers[0x09].store(data, std::memory_order_relaxed);
            break;

        case 0x0D:
            _registers[0x08].store(data & 0x3F, std::memory_order_relaxed);
            break;

        default:
            _logger->LogWarning("Unimplemented port write 0x%02x to 0x%02x in %s", data, port, "Smc5027Emulator");
            break;
    }
}

void Smc5027Emulator::Reset()
{
    _logger->LogTrace("Resetting %s", "Smc5027Emulator");

    for(
        uint8_t registerIndex = 0;
        registerIndex < REGISTER_COUNT;
        registerIndex++
    )
    {
        _registers[registerIndex].store(0x00, std::memory_order_relaxed);
    }
}

uint8_t Smc5027Emulator::GetCursorX()
{
    return Read(0x09);
}

uint8_t Smc5027Emulator::GetCursorY()
{
    return Read(0x08);
}

uint8_t Smc5027Emulator::FirstDisplayRow() {
    constexpr uint8_t rows = ROW_COUNT - 1;

    const uint8_t curEnd = _registers[0x06].load(std::memory_order_relaxed) & 0x1F;

    return (curEnd + 1) % (rows + 1);
}

uint8_t Smc5027Emulator::ConvertPort(uint8_t port)
{
    return port & 0x0F;
}

shared_ptr<ISmc5027Emulator> Smc5027Emulator::Creator(IPluginManager* manager)
{
    return make_shared<Smc5027Emulator>(manager->GetLogger());
}
