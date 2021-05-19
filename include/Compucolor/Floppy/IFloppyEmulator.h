
#pragma once

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IOutputDevice.h>
#include <Compucolor/Common/IResetable.h>

class IFloppyEmulator: public IEmulatable, public IOutputDevice, public IResetable {
    public:
        virtual ~IFloppyEmulator() {}

        virtual void Select(bool selected, bool write, uint8_t stepper) = 0;
};