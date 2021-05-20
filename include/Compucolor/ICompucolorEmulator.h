#pragma once

#include <Compucolor/Common/ISystem.h>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

namespace Compucolor
{
    class ICompucolorEmulator: public Common::ISystem {
        public:
            ICompucolorEmulator() {}

            virtual ~ICompucolorEmulator() {}

            virtual void OnKeyUp(Keyboard::CompucolorIIKey key) = 0;

            virtual void OnKeyDown(Keyboard::CompucolorIIKey key) = 0;
    };
}