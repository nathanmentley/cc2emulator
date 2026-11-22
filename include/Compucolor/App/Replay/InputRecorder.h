#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include <boost/json.hpp>

#include <Compucolor/App/Replay/ReplayBundle.h>
#include <Compucolor/App/Replay/ReplayEvent.h>

namespace Compucolor::Impl::App::Replay
{
    // Collects emulated-key transitions while a GUI session is being recorded.
    // Input arrives on the UI thread, so the event list is mutex-protected.
    class InputRecorder {
        public:
            void Start(const boost::json::value& state, uint64_t startNanoseconds);

            // Records one transition. Timestamps are clamped to be non-decreasing
            // because emulated time is published by another thread.
            void Record(
                bool down,
                Compucolor::Keyboard::CompucolorIIKey key,
                uint64_t nowNanoseconds
            );

            void Stop();
            [[nodiscard]] bool IsRecording() const;

            // Throws if no recording is active. The returned bundle is ready to
            // serialize and replay with the headless runner.
            [[nodiscard]] ReplayBundle BuildBundle(
                uint64_t stopNanoseconds,
                std::string createdBy = {}
            ) const;

        private:
            mutable std::mutex _mutex;
            bool _recording = false;
            boost::json::value _state{};
            uint64_t _startNanoseconds = 0;
            uint64_t _lastNanoseconds = 0;
            std::vector<ReplayEvent> _events;
    };
}
