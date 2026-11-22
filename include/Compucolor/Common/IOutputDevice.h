#pragma once

#include <cstdint>

namespace Compucolor::Common
{
    // Device that can be written through an 8080 I/O port.
    class IOutputDevice {
        public:
            virtual ~IOutputDevice() = default;

            virtual void Write(uint8_t port, uint8_t data) = 0;
    };
}
