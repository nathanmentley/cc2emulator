#pragma once

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IResetable.h>

#include <Compucolor/Keyboard/CompucolorIIKey.h>
#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Keyboard
{
    // 17-row x 8-bit active-low keyboard matrix. The TMS5501 reads one row at a
    // time on port 1; row 16 holds modifier keys. Short host taps are held for
    // one scan so they cannot fall between two keyboard reads.
    class IKeyboardEmulator: public Common::IPlugin, public Common::IEmulatable, public Common::IInputDevice, public Common::IResetable {
        public:
            ~IKeyboardEmulator() override = default;

            virtual void OnKeyUp(CompucolorIIKey key) = 0;

            virtual void OnKeyDown(CompucolorIIKey key) = 0;
    };
}
