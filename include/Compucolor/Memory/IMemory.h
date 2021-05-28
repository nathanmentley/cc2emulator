#pragma once

#include <stdint.h>

#include <boost/config.hpp>

#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Memory
{
    class BOOST_SYMBOL_VISIBLE IMemory: public Common::IPlugin {
        public:
            virtual ~IMemory() {}

            virtual uint8_t GetByte(uint16_t address) = 0;
            virtual void SetByte(uint16_t address, uint8_t data) = 0;
    };
}