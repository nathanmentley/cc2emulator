#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include <Compucolor/Common/IPluginManager.h>
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
                std::shared_ptr<Logger::ILogger> logger,
                std::shared_ptr<Intel8080::IIntel8080Emulator> intel8080,
                std::shared_ptr<Keyboard::IKeyboardEmulator> keyboard,
                std::shared_ptr<Floppy::IFloppyEmulator> floppy1,
                std::shared_ptr<Floppy::IFloppyEmulator> floppy2
            );

            void Start() override;
            void Stop() override;
            void Reset() override;

            void Step() override;
            void TriggerExternalSensor() override;

            uint8_t Read(uint8_t port) override;

            void Write(uint8_t port, uint8_t data) override;

            void OnInterruptConsumed(uint8_t opcode) override;

            static std::shared_ptr<ITms5501Emulator> Creator(Common::IPluginManager* manager);
        private:
            std::shared_ptr<Logger::ILogger> _logger;
            std::unique_ptr<Tms5501EmulatorContext> _context;
            std::shared_ptr<Intel8080::IIntel8080Emulator> _intel8080;
            std::shared_ptr<Keyboard::IKeyboardEmulator> _keyboard;
            std::shared_ptr<Floppy::IFloppyEmulator> _floppy1;
            std::shared_ptr<Floppy::IFloppyEmulator> _floppy2;
            uint8_t _outport;

            void SetOutport(uint8_t value);
            void ClearIntAddr(uint8_t op);
            void CheckInterruptStatus();

            [[nodiscard]] bool IsSerialSelected() const;
            [[nodiscard]] bool IsFloppy1Selected() const;
            [[nodiscard]] bool IsFloppy2Selected() const;
            [[nodiscard]] uint8_t GetIntAddr() const;

            static uint8_t ConvertPort(uint8_t port);
            static bool IsBitSet(uint8_t data, uint8_t pos);
    };
}