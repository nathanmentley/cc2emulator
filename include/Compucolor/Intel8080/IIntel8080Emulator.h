#pragma once

#include <cstdint>
#include <memory>

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IResetable.h>
#include <Compucolor/Common/IPlugin.h>

#include <Compucolor/Debug/IDebugSink.h>

#include <Compucolor/Intel8080/IIntel8080Bus.h>

namespace Compucolor::Intel8080
{
    // Wrapper around the vendored 8080 core. A bus must be installed with
    // SetBus() before stepping; memory and port accesses are delegated to it
    // and to the configured IMemory instance.
    class IIntel8080Emulator: public Common::IPlugin, public Common::IEmulatable, public Common::IResetable {
        public:
            ~IIntel8080Emulator() override = default;

            // Installs the I/O bus. Called by the machine when it is constructed.
            virtual void SetBus(std::shared_ptr<IIntel8080Bus> intel8080Bus) = 0;

            // Installs an optional instruction recorder.
            virtual void SetDebugSink(std::shared_ptr<Debug::IDebugSink> sink) = 0;

            // Returns the live CPU state. Call from the emulator thread only;
            // the host publishes a snapshot through IDebugSink for the UI.
            [[nodiscard]] virtual Debug::CpuState GetState() const = 0;

            // Latches an RST opcode (0xC7..0xFF) to present to the CPU on its next step.
            // The TMS5501 is the only production source of interrupt opcodes.
            virtual void RegisterInterrupt(uint8_t opcode) = 0;

            virtual bool IsInterruptPending() = 0;

            virtual uint8_t GetPendingInterrupt() = 0;

            // Executes one instruction and returns the number of cycles it took.
            // The current implementation reports at least four cycles.
            virtual uint64_t Step() = 0;
    };
}
