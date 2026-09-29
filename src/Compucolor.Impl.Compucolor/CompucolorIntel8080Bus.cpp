#include "CompucolorIntel8080Bus.h"

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
    // TMS5501 I/O chip
    if (port <= 0x1F) {
        return _tms5501->Read(port & 0x0F);
    }

    return port;
}

void Compucolor::Impl::CompucolorII::CompucolorIntel8080Bus::Write(const uint8_t port, const uint8_t data)
{
    // TMS5501 I/O chip
    if (port <= 0x1F) {
        return _tms5501->Write(port & 0x0F, data);
    }

    // SMC 5027 CRT chip
    if (port >= 0x60 && port <= 0x7F) {
        return _smc5027->Write(port & 0x0F, data);
    }
}