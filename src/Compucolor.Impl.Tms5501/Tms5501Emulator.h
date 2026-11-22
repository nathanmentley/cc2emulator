#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#include <Compucolor/Common/IAudioOutput.h>
#include <Compucolor/Common/IPluginManager.h>
#include <Compucolor/Floppy/IFloppyEmulator.h>
#include <Compucolor/Intel8080/IIntel8080Emulator.h>
#include <Compucolor/Keyboard/IKeyboardEmulator.h>
#include <Compucolor/Logger/ILogger.h>
#include <Compucolor/Tms5501/ITms5501Emulator.h>

#include "Tms5501EmulatorContext.h"

namespace Compucolor::Impl::Tms5501 {
    // Port-level TMS5501 emulation: keyboard rows, discrete commands, baud rate,
    // serial transmit, outport decode, interrupt mask/RST vectors and five timers.
    // The serial line drives SoundWare audio; floppy bytes never raise RX interrupts.
    class Tms5501Emulator: public Compucolor::Tms5501::ITms5501Emulator {
        public:
            Tms5501Emulator(
                std::shared_ptr<Compucolor::Logger::ILogger> logger,
                std::shared_ptr<Compucolor::Intel8080::IIntel8080Emulator> intel8080,
                std::shared_ptr<Compucolor::Keyboard::IKeyboardEmulator> keyboard,
                std::shared_ptr<Compucolor::Floppy::IFloppyEmulator> floppy1,
                std::shared_ptr<Compucolor::Floppy::IFloppyEmulator> floppy2,
                std::shared_ptr<Compucolor::Common::IAudioOutput> audio
            );

            void Start() override;
            void Stop() override;
            std::unique_ptr<boost::json::value> SaveState() override;
            void LoadState(std::shared_ptr<boost::json::value>) override;

            // Clears interrupt/timer state and releases the TXD audio line.
            void Reset() override;

            // Decrements timer counters and registers one due interrupt with the CPU.
            void Step() override;

            // Advances selected drive and audio time; completes pending serial transfers.
            void StepFloppy(uint64_t elapsedNanoseconds) override;

            // Latches the external sensor interrupt (0x04).
            void TriggerExternalSensor() override;

            uint8_t Read(uint8_t port) override;

            // Port 4's break bit toggles the SoundWare TXD line with a timestamp.
            void Write(uint8_t port, uint8_t data) override;

            // Clears the latched source matching an acknowledged RST opcode.
            void OnInterruptConsumed(uint8_t opcode) override;

            void SetEmulatedTime(uint64_t nanoseconds) override;

            // Captures the register file and decoded select/interrupt lines for
            // the host; timers are copied up to the size of the snapshot array.
            [[nodiscard]] Compucolor::Debug::Tms5501State GetDebugState() const override;

            static std::shared_ptr<ITms5501Emulator> Creator(Compucolor::Common::IPluginManager* manager);
        private:
            std::shared_ptr<Compucolor::Logger::ILogger> _logger;
            std::unique_ptr<Tms5501EmulatorContext> _context;
            std::shared_ptr<Compucolor::Intel8080::IIntel8080Emulator> _intel8080;
            std::shared_ptr<Compucolor::Keyboard::IKeyboardEmulator> _keyboard;
            std::shared_ptr<Compucolor::Floppy::IFloppyEmulator> _floppy1;
            std::shared_ptr<Compucolor::Floppy::IFloppyEmulator> _floppy2;
            std::shared_ptr<Compucolor::Common::IAudioOutput> _audio;
            uint64_t _emulatedNanoseconds = 0;
            bool _audioLevel = false;

            uint8_t _outport;
            uint64_t _floppyTransmitNanosecondsRemaining = 0;

            // Decodes drive select, write enable and stepper phase (TMS5501 port 7).
            void SetOutport(uint8_t value);
            void CompleteFloppyTransmit();
            void ClearIntAddr(uint8_t op);
            // Latches at most one pending interrupt as an RST opcode for the CPU.
            void CheckInterruptStatus();

            [[nodiscard]] bool IsSerialSelected() const;
            [[nodiscard]] bool IsFloppy1Selected() const;
            [[nodiscard]] bool IsFloppy2Selected() const;
            // Returns the RST opcode for the lowest-numbered unmasked pending interrupt.
            [[nodiscard]] uint8_t GetIntAddr() const;

            static uint8_t ConvertPort(uint8_t port);
            static bool IsBitSet(uint8_t data, uint8_t pos);
    };
}
