#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "Compucolor.Intel8080/IIntel8080Emulator.h"
#include "Compucolor.Keyboard/IKeyboardEmulator.h"
#include "Compucolor.Tms5501/ITms5501Emulator.h"
#include "Tms5501EmulatorContext.h"

class Tms5501Emulator: public ITms5501Emulator {
    public:
        Tms5501Emulator(
            std::shared_ptr<IIntel8080Emulator> intel8080,
            std::shared_ptr<IKeyboardEmulator> keyboard
        );

        virtual void Start() override;
        virtual void Stop() override;

        virtual uint8_t Read(uint8_t port) override;

        virtual void Write(uint8_t port, uint8_t data) override;

        virtual void Reset() override;
    private:
        std::unique_ptr<Tms5501EmulatorContext> _context;
        std::shared_ptr<IIntel8080Emulator> _intel8080;
        std::atomic<bool> _isRunning;
        std::shared_ptr<IKeyboardEmulator> _keyboard;
        uint8_t _outport;
        std::thread _thread;
        std::thread _keyboardThread;

        uint8_t ConvertPort(uint8_t port);

        void SetOutport(uint8_t value);

        uint8_t GetIntAddr();
        void ClearIntAddr(uint8_t op);
        bool IsBitSet(uint8_t data, uint8_t pos);
        void CheckInterruptStatus();
};