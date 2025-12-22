#pragma once

namespace Compucolor::Common
{
    class IOutputDevice {
        public:
            virtual ~IOutputDevice() = default;
    
            virtual void Write(uint8_t port, uint8_t data) = 0;
    };
}