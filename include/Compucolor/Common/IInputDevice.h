#pragma once

#include <cstdint>

namespace Compucolor::Common
{
    // Device that can be read through an 8080 I/O port. Port values are
    // normally masked or decoded by the bus before reaching the device.
    class IInputDevice {
        public:
            virtual ~IInputDevice() = default;

            virtual uint8_t Read(uint8_t port) = 0;
    };
}
