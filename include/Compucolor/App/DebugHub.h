#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include <Compucolor/Debug/IDebugSink.h>
#include <Compucolor/Debug/IReplayTickSink.h>

#include <Compucolor/App/IRecordableDebugSink.h>

namespace Compucolor::Impl::App
{
    // Fan-out sink used by the host. Recording is gated by the hub's master
    // switch, while state snapshots are always forwarded so the UI can display
    // live state with recording turned off.
    class DebugHub final:
        public Compucolor::Debug::IDebugSink,
        public Compucolor::Debug::IReplayTickSink {
        public:
            void AddSink(std::shared_ptr<Compucolor::Debug::IDebugSink> sink);
            void SetEnabled(bool enabled);

            [[nodiscard]] bool IsEnabled() const override;

            void SetCpuState(const Compucolor::Debug::CpuState& state) override;
            void OnTickBoundary(uint64_t emulatedNanoseconds) override;

            void SetCurrentPc(uint16_t pc) override;
            [[nodiscard]] uint16_t GetCurrentPc() const override;

            void OnInstruction(const Compucolor::Debug::InstructionEvent& event) override;
            void OnPort(const Compucolor::Debug::PortEvent& event) override;

            void SetTms5501State(const Compucolor::Debug::Tms5501State& state) override;
            void SetSmc5027State(const Compucolor::Debug::Smc5027State& state) override;

        private:
            [[nodiscard]] std::vector<std::shared_ptr<Compucolor::Debug::IDebugSink>> CopySinks() const;

            std::atomic<bool> _enabled{false};
            std::atomic<uint16_t> _currentPc{0};

            mutable std::mutex _sinkMutex;
            std::vector<std::shared_ptr<Compucolor::Debug::IDebugSink>> _sinks;
    };
}
