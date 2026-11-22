#pragma once

#include <cstdint>

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IOutputDevice.h>
#include <Compucolor/Common/IResetable.h>
#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Tms5501
{
    // TMS5501 timer/serial/keyboard/floppy I/O chip, accessed on ports 0x00-0x0F.
    // Read ports: 0 RX data, 1 keyboard matrix row, 2 interrupt acknowledge, 3 status.
    // Write ports: 4 discrete command (TXD break bit), 5 baud, 6 transmit,
    // 7 outport (drive select/write enable/stepper), 8 interrupt mask, 9-D timers.
    class ITms5501Emulator: public Common::IPlugin, public Common::IEmulatable, public Common::IInputDevice, public Common::IOutputDevice, public Common::IResetable {
        public:
            ~ITms5501Emulator() override = default;

            // Decrements the five timer counters and latches due interrupts.
            // Called by the machine every 128 CPU instructions.
            virtual void Step() = 0;

            // Advances the selected floppy and audio time by elapsed CPU time.
            // Received floppy bytes do not raise the serial RX interrupt; doing so
            // would let the ROM's RST4 handler corrupt typed console input.
            virtual void StepFloppy(uint64_t elapsedNanoseconds) = 0;

            // Latches the external sensor interrupt (used for the CRT vertical interval).
            virtual void TriggerExternalSensor() = 0;

            // Clears the interrupt source whose RST opcode the CPU has consumed.
            virtual void OnInterruptConsumed(uint8_t opcode) = 0;

            // Emulated time at the start of the CPU instruction about to run; stamps audio edges.
            virtual void SetEmulatedTime(uint64_t nanoseconds) = 0;
    };
}
