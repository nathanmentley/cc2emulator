#pragma once

#include <memory>

#include "Compucolor.Keyboard/IKeyboardEmulator.h"

#include "Compucolor.Tms5501/ITms5501Emulator.h"

class Tms5501Emulator: public ITms5501Emulator {
    public:
        Tms5501Emulator(
            std::shared_ptr<IKeyboardEmulator> keyboard
        );

        virtual void Start() override;
        virtual void Stop() override;

        virtual uint8_t Read(uint8_t port) override;

        virtual void Write(uint8_t port, uint8_t data) override;

        virtual void Reset() override;
    private:
        std::shared_ptr<IKeyboardEmulator> _keyboard;
        uint8_t _outport;

        uint8_t ConvertPort(uint8_t port);

        void SetOutport(uint8_t value);
};