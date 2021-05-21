#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include <Compucolor/Floppy/IFloppyEmulator.h>
#include <Compucolor/Intel8080/IIntel8080Emulator.h>
#include <Compucolor/Keyboard/IKeyboardEmulator.h>
#include <Compucolor/Logger/ILogger.h>
#include <Compucolor/Tms5501/ITms5501Emulator.h>

#include "Tms5501EmulatorContext.h"

namespace Compucolor::Impl::Tms5501 {
    class Tms5501Emulator: public Compucolor::Tms5501::ITms5501Emulator {
        public:
            Tms5501Emulator(
                std::shared_ptr<Compucolor::Logger::ILogger> logger,
                std::shared_ptr<Compucolor::Intel8080::IIntel8080Emulator> intel8080,
                std::shared_ptr<Compucolor::Keyboard::IKeyboardEmulator> keyboard,
                std::shared_ptr<Compucolor::Floppy::IFloppyEmulator> floppy1,
                std::shared_ptr<Compucolor::Floppy::IFloppyEmulator> floppy2
            );

            virtual void Start() override;
            virtual void Stop() override;

            virtual uint8_t Read(uint8_t port) override;

            virtual void Write(uint8_t port, uint8_t data) override;

            virtual void Reset() override;
        private:
            std::shared_ptr<Compucolor::Logger::ILogger> _logger;
            std::unique_ptr<Compucolor::Impl::Tms5501::Tms5501EmulatorContext> _context;
            std::shared_ptr<Compucolor::Intel8080::IIntel8080Emulator> _intel8080;
            std::atomic<bool> _isRunning;
            std::shared_ptr<Compucolor::Keyboard::IKeyboardEmulator> _keyboard;
            std::shared_ptr<Compucolor::Floppy::IFloppyEmulator> _floppy1;
            std::shared_ptr<Compucolor::Floppy::IFloppyEmulator> _floppy2;
            uint8_t _outport;
            std::thread _thread;
            std::thread _keyboardThread;

            uint8_t ConvertPort(uint8_t port);

            void SetOutport(uint8_t value);

            bool IsSerialSelected();
            bool IsFloppy1Selected();
            bool IsFloppy2Selected();
            uint8_t GetIntAddr();
            void ClearIntAddr(uint8_t op);
            bool IsBitSet(uint8_t data, uint8_t pos);
            void CheckInterruptStatus();
    };
}