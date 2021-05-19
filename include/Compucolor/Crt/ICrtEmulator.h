#pragma once

#include <Compucolor/Common/IDisplay.h>
#include <Compucolor/Common/IEmulatable.h>

namespace Compucolor::Crt
{
    class ICrtEmulator: public IEmulatable {
        public:
            virtual ~ICrtEmulator() {}

            virtual void SetDisplay(IDisplay* display) = 0;
    };
}