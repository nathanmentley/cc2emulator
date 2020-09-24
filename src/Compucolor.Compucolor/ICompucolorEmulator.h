#pragma once

#include "Compucolor.Core/IDisplay.h"
#include "Compucolor.Core/ISystem.h"

class ICompucolorEmulator: public ISystem {
    public:
        ICompucolorEmulator() {}

        virtual ~ICompucolorEmulator() {}

        virtual void SetDisplay(IDisplay* display) = 0;
};