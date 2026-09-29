#pragma once

#include <cstdint>

namespace Compucolor::Common
{
    class IInputDevice {
        public:
            virtual ~IInputDevice() = default;
    
            virtual uint8_t Read(uint8_t port) = 0;
    };
}