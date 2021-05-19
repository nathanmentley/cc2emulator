
#pragma once

#include <memory>

#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IOutputDevice.h>

namespace Compucolor::Intel8080
{
    class IIntel8080Bus: public IInputDevice, public IOutputDevice
    {
    };
}