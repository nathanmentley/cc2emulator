#pragma once

#include <atomic>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <string>

#include <boost/json.hpp>

#include <Compucolor/Debug/IDebugSink.h>

#include "IRecordableDebugSink.h"

namespace Compucolor::Impl::App::Sfml
{
    // Appends newline-delimited JSON records to a file. Each record gets a local
    // sequence number because upstream events may not carry a useful one.
    class JsonlDebugSink final:
        public Compucolor::Debug::IDebugSink,
        public IRecordableDebugSink {
        public:
            explicit JsonlDebugSink(std::string path);
            ~JsonlDebugSink() override;

            JsonlDebugSink(const JsonlDebugSink&) = delete;
            JsonlDebugSink& operator=(const JsonlDebugSink&) = delete;

            void SetEnabled(bool enabled) override;
            [[nodiscard]] bool IsEnabled() const override;

            void SetCpuState(const Compucolor::Debug::CpuState& state) override;

            void SetCurrentPc(uint16_t pc) override;
            [[nodiscard]] uint16_t GetCurrentPc() const override;

            void OnInstruction(const Compucolor::Debug::InstructionEvent& event) override;
            void OnPort(const Compucolor::Debug::PortEvent& event) override;

            void SetTms5501State(const Compucolor::Debug::Tms5501State& state) override;
            void SetSmc5027State(const Compucolor::Debug::Smc5027State& state) override;

        private:
            void WriteLine(boost::json::object line);

            [[nodiscard]] static boost::json::string Hex(uint64_t value, int width);

            std::string _path;
            std::ofstream _stream;
            std::mutex _mutex;
            std::atomic<bool> _enabled{true};
            std::atomic<uint16_t> _currentPc{0};
            uint64_t _nextSequence = 1;
    };
}
