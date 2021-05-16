#pragma once

#include "Compucolor.Core/IDisplay.h"
#include "Compucolor.Core/IEmulatable.h"

class ICrtEmulator: public IEmulatable {
    public:
        virtual ~ICrtEmulator() {}

        virtual void SetDisplay(IDisplay* display) = 0;
};