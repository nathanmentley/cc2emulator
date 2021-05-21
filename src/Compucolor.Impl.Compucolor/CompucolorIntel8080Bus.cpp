#include "CompucolorIntel8080Bus.h"

Compucolor::Impl::CompucolorII::CompucolorIntel8080Bus::CompucolorIntel8080Bus(
    std::shared_ptr<Compucolor::Smc5027::ISmc5027Emulator> smc5027,
    std::shared_ptr<Compucolor::Tms5501::ITms5501Emulator> tms5501
):
    _smc5027(smc5027),
    _tms5501(tms5501)
{
}

uint8_t Compucolor::Impl::CompucolorII::CompucolorIntel8080Bus::Read(uint8_t port)
{
    if (port >= 0x00 && port <= 0x1F) {
        return _tms5501->Read(port & 0x0F);
    }

    return port;
}

void Compucolor::Impl::CompucolorII::CompucolorIntel8080Bus::Write(uint8_t port, uint8_t data)
{
    // TMS5501 I/O chip
    if ((port >= 0x00 && port <= 0x0F) || (port >= 0x10 && port <= 0x1F)) {
        return _tms5501->Write(port & 0x0F, data);
    }

    // SMC 5027 CRT chip
    if ((port >= 0x60 && port <= 0x6F) || (port >= 0x70 && port <= 0x7F)) {
        return _smc5027->Write(port & 0x0F, data);
    }
}