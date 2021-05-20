#pragma once

#include <Compucolor/Common/IDisplay.h>
#include <Compucolor/Common/IEmulatable.h>

namespace Compucolor::Crt
{
    class ICrtEmulator: public Common::IEmulatable {
        public:
            virtual ~ICrtEmulator() {}
    };
}