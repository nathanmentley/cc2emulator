#pragma once

#include <Compucolor/Common/IDisplay.h>
#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Crt
{
    class ICrtEmulator: public Common::IPlugin, public Common::IEmulatable {
        public:
            virtual ~ICrtEmulator() {}
    };
}