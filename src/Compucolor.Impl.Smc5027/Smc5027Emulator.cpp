#include "Smc5027Emulator.h"

Compucolor::Impl::Smc5027::Smc5027Emulator::Smc5027Emulator(
    std::shared_ptr<Logger::ILogger> logger
):
    _logger(logger),
    _registers(std::vector<uint8_t>(Smc5027Emulator::REGISTER_COUNT))
{
    Reset();
}

void Compucolor::Impl::Smc5027::Smc5027Emulator::Start()
{
    _logger->LogTrace(
        "Starting %s",
        "Smc5027Emulator"
    );

    Reset();
}

void Compucolor::Impl::Smc5027::Smc5027Emulator::Stop()
{
    _logger->LogTrace(
        "Stopping %s",
        "Smc5027Emulator"
    );

    Reset();
}

uint8_t Compucolor::Impl::Smc5027::Smc5027Emulator::Read(uint8_t port)
{
    uint8_t data = 0x00;

    switch (ConvertPort(port))
    {
        case 0x08:
            data = _registers[0x08];
            break;

        case 0x09:
            data = _registers[0x09];
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

void Compucolor::Impl::Smc5027::Smc5027Emulator::Write(uint8_t port, uint8_t data)
{
    _logger->LogTrace(
        "Writing data: 0x%02x to port: 0x%02x in %s",
        data,
        port,
        "Smc5027Emulator"
    );

    switch (ConvertPort(port))
    {
        /*
        case 0x06:
            _registers[0x06] = data;
            break;
        case 0x0B:
            _registers[0x06] = (_registers[0x06] + 1) % 32;
            break;
        */
        case 0x0C:
            _registers[0x09] = data;
            break;
        case 0x0D:
            _registers[0x08] = data & 0x3F;
            break;
    }
}

void Compucolor::Impl::Smc5027::Smc5027Emulator::Reset()
{
    _logger->LogTrace(
        "Resetting %s",
        "Smc5027Emulator"
    );

    for(
        uint8_t registerIndex = 0;
        registerIndex < Smc5027Emulator::REGISTER_COUNT;
        registerIndex++
    )
    {
        _registers[registerIndex] = 0x00;
    }
}

uint8_t Compucolor::Impl::Smc5027::Smc5027Emulator::GetCursorX()
{
    return Read(0x09);
}

uint8_t Compucolor::Impl::Smc5027::Smc5027Emulator::GetCursorY()
{
    return Read(0x08);
}

uint8_t Compucolor::Impl::Smc5027::Smc5027Emulator::ConvertPort(uint8_t port)
{
    return port & 0x0F;
}