
#pragma once

#include <Compucolor.Core/IEmulatable.h>
#include <Compucolor.Core/IOutputDevice.h>
#include <Compucolor.Core/IResetable.h>

class IFloppyEmulator: public IEmulatable, public IOutputDevice, public IResetable {
    public:
        virtual ~IFloppyEmulator() {}

        virtual void Select(bool selected, bool write, uint8_t stepper) = 0;
};