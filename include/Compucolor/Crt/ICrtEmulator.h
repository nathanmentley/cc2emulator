#pragma once

#include <Compucolor/Common/IDisplay.h>
#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Crt
{
    // Rebuilds the 384x256 framebuffer from video RAM and publishes it to IDisplay.
    class ICrtEmulator: public Common::IPlugin, public Common::IEmulatable {
        public:
            ~ICrtEmulator() override = default;

            // Selects the embedded lowercase UF6 font instead of the standard one.
            virtual void SetLowercaseCharset(bool lowercase) = 0;
    };
}
