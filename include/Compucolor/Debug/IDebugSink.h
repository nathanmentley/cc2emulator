#pragma once

#include <array>
#include <cstdint>
#include <memory>

namespace Compucolor::Debug
{
    // Snapshot of the 8080's externally visible state. Emulated time is filled
    // in by the machine, which owns the pacing clock.
    struct CpuState {
        uint64_t cycles = 0;
        uint64_t emulatedNanoseconds = 0;

        uint16_t pc = 0;
        uint16_t sp = 0;

        uint8_t a = 0;
        uint8_t b = 0;
        uint8_t c = 0;
        uint8_t d = 0;
        uint8_t e = 0;
        uint8_t h = 0;
        uint8_t l = 0;

        uint8_t interruptVector = 0;
        uint8_t interruptDelay = 0;

        bool sf = false;
        bool zf = false;
        bool hf = false;
        bool pf = false;
        bool cf = false;
        bool iff = false;
        bool halted = false;
        bool interruptPending = false;
    };

    // One executed instruction. The three opcode bytes are captured before the
    // step so a disassembler can decode the instruction afterwards.
    struct InstructionEvent {
        uint64_t sequence = 0;
        CpuState before;
        std::array<uint8_t, 3> bytes{};
        uint64_t cycles = 0;
    };

    // One I/O access performed by the bus while an instruction was executing.
    struct PortEvent {
        uint64_t sequence = 0;
        uint16_t pc = 0;
        bool isWrite = false;
        uint8_t port = 0;
        uint8_t value = 0;
    };

    // Host-safe snapshot of the TMS5501 register file and decoded line state.
    struct Tms5501State {
        uint8_t status = 0;
        uint8_t intMask = 0;
        uint8_t intStatus = 0;
        uint8_t outport = 0;
        uint8_t rate = 0;
        uint8_t dscCmd = 0;
        uint8_t rxData = 0;
        uint8_t txData = 0;
        uint8_t txDataCount = 0;
        std::array<uint8_t, 5> timers{};

        bool audioLevel = false;
        bool serialSelected = false;
        bool floppy1Selected = false;
        bool floppy2Selected = false;
        uint8_t pendingInterrupt = 0;
    };

    // Host-safe snapshot of the SMC5027 registers exposed by the production bus.
    struct Smc5027State {
        uint8_t cursorX = 0;
        uint8_t cursorY = 0;
        uint8_t firstDisplayRow = 0;
    };

    // Host-provided recorder for optional debug instrumentation. Implementations
    // must be safe to call from emulator threads; the host is responsible for
    // publishing snapshots back to the UI thread.
    class IDebugSink {
        public:
            virtual ~IDebugSink() = default;

            [[nodiscard]] virtual bool IsEnabled() const = 0;

            // Published at most once per machine tick, not once per instruction.
            virtual void SetCpuState(const CpuState& state) = 0;

            // Called immediately before Step() so port events can be attributed
            // to the instruction that caused them.
            virtual void SetCurrentPc(uint16_t pc) = 0;

            [[nodiscard]] virtual uint16_t GetCurrentPc() const = 0;

            virtual void OnInstruction(const InstructionEvent& event) = 0;

            virtual void OnPort(const PortEvent& event) = 0;

            // Peripheral snapshots, published once per machine tick.
            virtual void SetTms5501State(const Tms5501State& state) = 0;

            virtual void SetSmc5027State(const Smc5027State& state) = 0;
    };
}
