#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include <Compucolor/Debug/IDebugSink.h>
#include <Compucolor/Memory/IMemory.h>

#include "DebugFilter.h"

namespace Compucolor::Impl::App::Sfml
{
    // Host-side debug recorder. Emulator threads append to bounded ring buffers;
    // the UI thread reads immutable copies through the snapshot getters.
    class DebugMonitor final: public Compucolor::Debug::IDebugSink {
        public:
            explicit DebugMonitor(std::shared_ptr<Compucolor::Memory::IMemory> memory);

            [[nodiscard]] bool IsEnabled() const override;
            void SetEnabled(bool enabled);

            void SetCpuState(const Compucolor::Debug::CpuState& state) override;

            void SetCurrentPc(uint16_t pc) override;
            [[nodiscard]] uint16_t GetCurrentPc() const override;

            void OnInstruction(const Compucolor::Debug::InstructionEvent& event) override;
            void OnPort(const Compucolor::Debug::PortEvent& event) override;

            void SetInstructionFilter(const DebugFilter& filter);
            void SetPortFilter(const DebugFilter& filter);
            [[nodiscard]] DebugFilter GetInstructionFilter() const;
            [[nodiscard]] DebugFilter GetPortFilter() const;

            [[nodiscard]] bool IsInstructionTriggered() const;
            [[nodiscard]] bool IsPortTriggered() const;
            void RearmTriggers();

            void SetTms5501State(const Compucolor::Debug::Tms5501State& state) override;
            void SetSmc5027State(const Compucolor::Debug::Smc5027State& state) override;

            [[nodiscard]] Compucolor::Debug::CpuState GetCpuState() const;
            [[nodiscard]] Compucolor::Debug::Tms5501State GetTms5501State() const;
            [[nodiscard]] Compucolor::Debug::Smc5027State GetSmc5027State() const;

            [[nodiscard]] std::vector<Compucolor::Debug::InstructionEvent> GetInstructions() const;
            [[nodiscard]] std::vector<Compucolor::Debug::PortEvent> GetPorts() const;

            [[nodiscard]] uint8_t ReadMemory(uint16_t address) const;
            void WriteMemory(uint16_t address, uint8_t value) const;

            void Clear();

        private:
            static constexpr std::size_t MaxEntries = 512;

            [[nodiscard]] static bool MatchesInstruction(
                const Compucolor::Debug::InstructionEvent& event,
                const DebugFilter& filter
            );
            [[nodiscard]] static bool MatchesPort(
                const Compucolor::Debug::PortEvent& event,
                const DebugFilter& filter
            );

            std::shared_ptr<Compucolor::Memory::IMemory> _memory;
            std::atomic<bool> _enabled{false};
            std::atomic<uint16_t> _currentPc{0};

            mutable std::mutex _mutex;
            Compucolor::Debug::CpuState _cpuState{};
            Compucolor::Debug::Tms5501State _tms5501State{};
            Compucolor::Debug::Smc5027State _smc5027State{};
            std::deque<Compucolor::Debug::InstructionEvent> _instructions;
            std::deque<Compucolor::Debug::PortEvent> _ports;
            DebugFilter _instructionFilter{};
            DebugFilter _portFilter{};
            bool _instructionTriggered = false;
            bool _portTriggered = false;
            uint64_t _nextSequence = 1;
    };
}
