#pragma once

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IOutputDevice.h>
#include <Compucolor/Common/IResetable.h>

class ITms5501Emulator: public IEmulatable, public IInputDevice, public IOutputDevice, public IResetable {
    public:
        virtual ~ITms5501Emulator() {}
};