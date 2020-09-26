#pragma once

#include "Compucolor.Core/IEmulatable.h"
#include "Compucolor.Core/IInputDevice.h"
#include "Compucolor.Core/IResetable.h"

class IKeyboardEmulator: public IEmulatable, public IInputDevice, public IResetable {
    public:
        virtual ~IKeyboardEmulator() {}
};