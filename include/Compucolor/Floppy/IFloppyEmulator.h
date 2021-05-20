
#pragma once

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IOutputDevice.h>
#include <Compucolor/Common/IResetable.h>

namespace Compucolor::Floppy
{
    class IFloppyEmulator: public Common::IEmulatable, public Common::IOutputDevice, public Common::IResetable {
        public:
            virtual ~IFloppyEmulator() {}

            virtual void Select(bool selected, bool write, uint8_t stepper) = 0;
    };
}