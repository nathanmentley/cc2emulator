#pragma once

#include <Compucolor/Common/IDisplay.h>
#include <Compucolor/Common/ISystem.h>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

class ICompucolorEmulator: public ISystem {
    public:
        ICompucolorEmulator() {}

        virtual ~ICompucolorEmulator() {}

        virtual void SetDisplay(IDisplay* display) = 0;

        virtual void OnKeyUp(CompucolorIIKey key) = 0;

        virtual void OnKeyDown(CompucolorIIKey key) = 0;
};