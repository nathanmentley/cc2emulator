#include "CompucolorIntel8080Bus.h"
#include "Roms.h"

#include <utility>

Compucolor::Impl::CompucolorII::CompucolorIntel8080Bus::CompucolorIntel8080Bus(
    std::shared_ptr<Smc5027::ISmc5027Emulator> smc5027,
    std::shared_ptr<Tms5501::ITms5501Emulator> tms5501
):
    _smc5027(std::move(smc5027)),
    _tms5501(std::move(tms5501))
{
}

uint8_t Compucolor::Impl::CompucolorII::CompucolorIntel8080Bus::Read(const uint8_t port)
{
    uint8_t value = port;

    // CRT timing PROM
    if (port >= 0x80 && port <= 0x9F)
    {
        value = get_crt_timing_rom()[port & 0x1F];
    }
    // TMS5501 I/O chip
    else if (port <= 0x1F)
    {
        value = _tms5501->Read(port & 0x0F);
    }

    if (_debugSink && _debugSink->IsEnabled())
    {
        Compucolor::Debug::PortEvent event;
        event.pc = _debugSink->GetCurrentPc();
        event.isWrite = false;
        event.port = port;
        event.value = value;
        _debugSink->OnPort(event);
    }

    return value;
}

void Compucolor::Impl::CompucolorII::CompucolorIntel8080Bus::Write(const uint8_t port, const uint8_t data)
{
    // TMS5501 I/O chip
    if (port <= 0x1F)
    {
        _tms5501->Write(port & 0x0F, data);
    }
    // SMC 5027 CRT chip
    else if (port >= 0x60 && port <= 0x7F)
    {
        _smc5027->Write(port & 0x0F, data);
    }

    if (_debugSink && _debugSink->IsEnabled())
    {
        Compucolor::Debug::PortEvent event;
        event.pc = _debugSink->GetCurrentPc();
        event.isWrite = true;
        event.port = port;
        event.value = data;
        _debugSink->OnPort(event);
    }
}

void Compucolor::Impl::CompucolorII::CompucolorIntel8080Bus::SetDebugSink(
    std::shared_ptr<Compucolor::Debug::IDebugSink> sink
)
{
    _debugSink = std::move(sink);
}