#pragma once

#include <cstdint>

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Memory
{
    // Byte-addressed 16-bit memory space (64 KiB). State is persisted as a JSON array.
    class IMemory: public Common::IPlugin, public Common::IEmulatable {
        public:
            virtual ~IMemory() = default;

            // Reads a byte. The implementation aliases 0x6000-0x6FFF to video RAM at 0x7000-0x7FFF.
            virtual uint8_t GetByte(uint16_t address) = 0;

            // Writes a byte, including through the same video RAM alias.
            virtual void SetByte(uint16_t address, uint8_t data) = 0;
    };
}
