#pragma once

#include "Compucolor.Core/IEmulatable.h"
#include "Compucolor.Core/IInputDevice.h"
#include "Compucolor.Core/IOutputDevice.h"
#include "Compucolor.Core/IResetable.h"

class ISmc5027Emulator: public IEmulatable, public IInputDevice, public IOutputDevice, public IResetable {
    public:
        virtual ~ISmc5027Emulator() {}
};