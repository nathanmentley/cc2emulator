#pragma once

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IResetable.h>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

namespace Compucolor::Keyboard
{
    class IKeyboardEmulator: public Common::IEmulatable, public Common::IInputDevice, public Common::IResetable {
        public:
            virtual ~IKeyboardEmulator() {}

            virtual void OnKeyUp(CompucolorIIKey key) = 0;

            virtual void OnKeyDown(CompucolorIIKey key) = 0;
    };
}