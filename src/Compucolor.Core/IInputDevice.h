
#pragma once

#include <memory>

class IInputDevice {
    public:
        virtual ~IInputDevice() {}

        virtual uint8_t Read(uint8_t port) = 0;
};