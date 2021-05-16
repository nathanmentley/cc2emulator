#pragma once

#include <memory>

class IOutputDevice {
    public:
        virtual ~IOutputDevice() {}

        virtual void Write(uint8_t port, uint8_t data) = 0;
};