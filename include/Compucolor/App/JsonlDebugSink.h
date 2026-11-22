#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <variant>

#include <boost/json.hpp>

#include <Compucolor/Debug/IDebugSink.h>

#include <Compucolor/App/IRecordableDebugSink.h>
#include <Compucolor/App/TraceFilterConfig.h>

namespace Compucolor::Impl::App
{
    // Appends newline-delimited JSON records to a file. Each record gets a local
    // sequence number because upstream events may not carry a useful one.
    //
    // Callbacks only copy small snapshots onto a queue; filtering, formatting
    // and file I/O happen on a worker thread so tracing does not slow the
    // emulator's wall-clock paced loop.
    class JsonlDebugSink final:
        public Compucolor::Debug::IDebugSink,
        public IRecordableDebugSink {
        public:
            explicit JsonlDebugSink(
                std::string path,
                TraceFilterConfig filters = {}
            );
            ~JsonlDebugSink() override;

            JsonlDebugSink(const JsonlDebugSink&) = delete;
            JsonlDebugSink& operator=(const JsonlDebugSink&) = delete;

            void SetEnabled(bool enabled) override;
            [[nodiscard]] bool IsEnabled() const override;

            // Waits for queued records to be written, then flushes the stream.
            void Flush();

            void SetCpuState(const Compucolor::Debug::CpuState& state) override;

            void SetCurrentPc(uint16_t pc) override;
            [[nodiscard]] uint16_t GetCurrentPc() const override;

            void OnInstruction(const Compucolor::Debug::InstructionEvent& event) override;
            void OnPort(const Compucolor::Debug::PortEvent& event) override;

            void SetTms5501State(const Compucolor::Debug::Tms5501State& state) override;
            void SetSmc5027State(const Compucolor::Debug::Smc5027State& state) override;

        private:
            struct ResolvedPortEvent {
                Compucolor::Debug::PortEvent event;
                uint16_t pc = 0;
            };

            using TraceRecord = std::variant<
                Compucolor::Debug::CpuState,
                Compucolor::Debug::InstructionEvent,
                ResolvedPortEvent,
                Compucolor::Debug::Tms5501State,
                Compucolor::Debug::Smc5027State
            >;

            template<typename T>
            void Enqueue(T value)
            {
                {
                    std::lock_guard<std::mutex> lock(_queueMutex);
                    _queue.emplace_back(std::move(value));
                    ++_pending;
                }

                _queueCv.notify_one();
            }

            void WorkerLoop();
            void ProcessRecord(const Compucolor::Debug::CpuState& state);
            void ProcessRecord(const Compucolor::Debug::InstructionEvent& event);
            void ProcessRecord(const ResolvedPortEvent& record);
            void ProcessRecord(const Compucolor::Debug::Tms5501State& state);
            void ProcessRecord(const Compucolor::Debug::Smc5027State& state);

            void WriteLine(boost::json::object line);

            [[nodiscard]] static boost::json::string Hex(uint64_t value, int width);

            std::string _path;
            TraceFilterConfig _filters;
            std::ofstream _stream;

            std::mutex _streamMutex;
            uint64_t _nextSequence = 1;

            std::mutex _queueMutex;
            std::condition_variable _queueCv;
            std::deque<TraceRecord> _queue;
            std::size_t _pending = 0;
            bool _stopping = false;
            std::thread _worker;

            std::atomic<bool> _enabled{true};
            std::atomic<uint16_t> _currentPc{0};
    };
}
