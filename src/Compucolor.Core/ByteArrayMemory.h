#pragma once

#include <vector>

#include <fruit/fruit.h>

#include "IMemory.h"

class ByteArrayMemory: public IMemory {
    public:
        INJECT(ByteArrayMemory());

        virtual uint8_t GetByte(uint16_t address) override;
        virtual void SetByte(uint16_t address, uint8_t data) override;

    private:
        std::vector<uint8_t> _data;
};