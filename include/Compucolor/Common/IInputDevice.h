#pragma once

#include <memory>

namespace Compucolor::Common
{
    class IInputDevice {
        public:
            virtual ~IInputDevice() {}
    
            virtual uint8_t Read(uint8_t port) = 0;
    };
}