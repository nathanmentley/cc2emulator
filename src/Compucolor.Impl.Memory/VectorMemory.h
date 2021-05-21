#pragma once

#include <vector>

#include <Compucolor/Memory/IMemory.h>

namespace Compucolor::Impl::Memory
{
    class VectorMemory: public Compucolor::Memory::IMemory {
        public:
            VectorMemory();

            virtual uint8_t GetByte(uint16_t address) override;
            virtual void SetByte(uint16_t address, uint8_t data) override;

        private:
            std::vector<uint8_t> _data;
    };
}