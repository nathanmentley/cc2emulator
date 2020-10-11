#pragma once

#include <Compucolor.Core/IEmulatable.h>
#include <Compucolor.Core/IInputDevice.h>
#include <Compucolor.Core/IResetable.h>

#include <Compucolor.Keyboard/CompucolorIIKey.h>

class IKeyboardEmulator: public IEmulatable, public IInputDevice, public IResetable {
    public:
        virtual ~IKeyboardEmulator() {}

        virtual void OnKeyUp(CompucolorIIKey key) = 0;

        virtual void OnKeyDown(CompucolorIIKey key) = 0;
};