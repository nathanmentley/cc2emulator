#pragma once

#include "Compucolor.Core/IDisplay.h"
#include "Compucolor.Core/IEmulatable.h"

class ICrtEmulator: public IEmulatable {
    public:
        virtual ~ICrtEmulator() {}

        virtual void RefreshDisplay() = 0;

        virtual void SetDisplay(IDisplay* display) = 0;
};