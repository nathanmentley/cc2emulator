#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>

#include <Compucolor/Debug/IDebugSink.h>
#include <Compucolor/Debug/IReplayTickSink.h>

#include <Compucolor/App/Replay/InputTimeline.h>
#include <Compucolor/App/Replay/ReplayBundle.h>
#include <Compucolor/App/Replay/ReplayEvent.h>

namespace Compucolor::Impl::App::Replay
{
    struct ReplayProgress {
        bool active = false;
        uint64_t currentNanoseconds = 0;
        uint64_t nextEventNanoseconds = std::numeric_limits<uint64_t>::max();
        uint64_t stopNanoseconds = 0;
        std::size_t appliedEvents = 0;
        std::size_t totalEvents = 0;
    };

    // Host-side debug sink that feeds a replay bundle's input timeline at the
    // start of each machine tick. Applying events before CPU work makes a key
    // contact visible to the ROM's keyboard scan even when tracing makes ticks
    // coarse. CPU snapshots still update progress reporting only.
    class ReplayInputSink final:
        public Compucolor::Debug::IDebugSink,
        public Compucolor::Debug::IReplayTickSink {
        public:
            using ApplyHandler = std::function<void(const ReplayEvent&)>;

            void Start(
                const ReplayBundle& bundle,
                ApplyHandler apply,
                uint16_t initialPc = 0
            );

            void Stop();
            [[nodiscard]] bool IsActive() const;
            [[nodiscard]] ReplayProgress GetProgress() const;

            [[nodiscard]] bool IsEnabled() const override { return true; }
            void SetCpuState(const Compucolor::Debug::CpuState& state) override;
            void OnTickBoundary(uint64_t emulatedNanoseconds) override;
            void SetCurrentPc(uint16_t pc) override;
            [[nodiscard]] uint16_t GetCurrentPc() const override;
            void OnInstruction(const Compucolor::Debug::InstructionEvent& event) override;
            void OnPort(const Compucolor::Debug::PortEvent& event) override;
            void SetTms5501State(const Compucolor::Debug::Tms5501State& state) override;
            void SetSmc5027State(const Compucolor::Debug::Smc5027State& state) override;

        private:
            mutable std::mutex _mutex;
            std::unique_ptr<InputTimeline> _timeline;
            ApplyHandler _apply;
            uint64_t _currentNanoseconds = 0;
            uint64_t _stopNanoseconds = 0;
            std::size_t _appliedEvents = 0;
            std::size_t _totalEvents = 0;
            uint16_t _pc = 0;
    };
}
