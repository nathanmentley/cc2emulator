#pragma once

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IOutputDevice.h>
#include <Compucolor/Common/IResetable.h>
#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Smc5027
{
    // SMC5027 CRT controller. Holds the scroll/start-row and cursor registers
    // written by the CPU on ports 0x60-0x6F; the CRT reads them each refresh.
    class ISmc5027Emulator: public Common::IPlugin, public Common::IEmulatable, public Common::IInputDevice, public Common::IOutputDevice, public Common::IResetable {
        public:
            ~ISmc5027Emulator() override = default;

            // Cursor column from register 9 (written on port 0x0C).
            virtual uint8_t GetCursorX() = 0;

            // Cursor row from register 8 (written on port 0x0D, masked to 6 bits).
            virtual uint8_t GetCursorY() = 0;

            // First visible video row: (register 6 & 0x1F) + 1, modulo 32.
            // Port 0x0B increments this register modulo 32 for hardware scrolling.
            virtual uint8_t FirstDisplayRow() = 0;
    };
}
