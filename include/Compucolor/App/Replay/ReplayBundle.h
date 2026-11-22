#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <boost/json.hpp>

#include <Compucolor/App/Replay/ReplayEvent.h>

namespace Compucolor::Impl::App::Replay
{
    // Versioned JSON bundle: a complete machine state plus emulated-time input
    // and the time at which replay should stop.
    class ReplayBundle {
        public:
            ReplayBundle(
                boost::json::value state,
                std::vector<ReplayEvent> input,
                uint64_t stopNanoseconds,
                std::string createdBy = {}
            );

            static ReplayBundle ParseFile(const std::string& path);
            static ReplayBundle Parse(
                std::string_view contents,
                std::string_view source = "replay bundle"
            );

            [[nodiscard]] const boost::json::value& State() const { return _state; }
            [[nodiscard]] const std::vector<ReplayEvent>& Input() const { return _input; }
            [[nodiscard]] uint64_t StopNanoseconds() const { return _stopNanoseconds; }
            [[nodiscard]] const std::string& CreatedBy() const { return _createdBy; }

            [[nodiscard]] boost::json::value ToJson() const;

        private:
            boost::json::value _state;
            std::vector<ReplayEvent> _input;
            uint64_t _stopNanoseconds = 0;
            std::string _createdBy;
    };
}
