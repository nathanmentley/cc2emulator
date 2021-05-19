#pragma once

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IResetable.h>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

class IKeyboardEmulator: public IEmulatable, public IInputDevice, public IResetable {
    public:
        virtual ~IKeyboardEmulator() {}

        virtual void OnKeyUp(CompucolorIIKey key) = 0;

        virtual void OnKeyDown(CompucolorIIKey key) = 0;
};