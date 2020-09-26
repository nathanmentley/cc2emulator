#pragma once

#include "Compucolor.Core/IEmulatable.h"
#include "Compucolor.Core/IInputDevice.h"
#include "Compucolor.Core/IOutputDevice.h"
#include "Compucolor.Core/IResetable.h"

class ITms5501Emulator: public IEmulatable, public IInputDevice, public IOutputDevice, public IResetable {
    public:
        virtual ~ITms5501Emulator() {}
};