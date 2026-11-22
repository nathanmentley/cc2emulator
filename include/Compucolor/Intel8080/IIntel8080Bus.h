#pragma once

#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IOutputDevice.h>

namespace Compucolor::Intel8080
{
    // I/O port abstraction for the 8080's IN/OUT instructions.
    // CompucolorIntel8080Bus routes ports to the TMS5501, SMC5027 and CRT timing PROM.
    class IIntel8080Bus: public Common::IInputDevice, public Common::IOutputDevice
    {
    };
}
