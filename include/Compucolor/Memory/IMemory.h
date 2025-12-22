#pragma once

#include <cstdint>

#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Memory
{
    class IMemory: public Common::IPlugin {
        public:
            virtual ~IMemory() = default;

            virtual uint8_t GetByte(uint16_t address) = 0;
            virtual void SetByte(uint16_t address, uint8_t data) = 0;
    };
}