
#pragma once

#include <memory>

#include "Compucolor.Core/IInputDevice.h"
#include "Compucolor.Core/IOutputDevice.h"

class IIntel8080Bus: public IInputDevice, public IOutputDevice
{

};