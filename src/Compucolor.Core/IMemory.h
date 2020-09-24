
#pragma once

class IMemory {
    public:
        virtual ~IMemory() {}

        virtual uint8_t GetByte(uint16_t address) = 0;
        virtual void SetByte(uint16_t address, uint8_t data) = 0;
};