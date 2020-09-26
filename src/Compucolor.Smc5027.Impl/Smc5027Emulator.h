#pragma once

#include <memory>
#include <vector>

#include "Compucolor.Smc5027/ISmc5027Emulator.h"

class Smc5027Emulator: public ISmc5027Emulator {
    public:
        Smc5027Emulator();

        virtual void Start() override;
        virtual void Stop() override;

        virtual uint8_t Read(uint8_t port) override;

        virtual void Write(uint8_t port, uint8_t data) override;

        virtual void Reset() override;
    private:
        std::vector<uint8_t> _registers;

        uint8_t ConvertPort(uint8_t port);
};