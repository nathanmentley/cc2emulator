#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include <Compucolor/App/Replay/ReplayEvent.h>

namespace Compucolor::Impl::App::Replay
{
    // Sorted replay input cursor. Events are applied once, in bundle order,
    // when emulated time reaches their timestamp.
    class InputTimeline {
        public:
            explicit InputTimeline(std::vector<ReplayEvent> events);

            // Applies every event whose timestamp is <= now. The callback must
            // forward the event to the emulated keyboard.
            void ApplyDue(
                uint64_t now,
                const std::function<void(const ReplayEvent&)>& apply
            );

            [[nodiscard]] bool HasPending() const;
            [[nodiscard]] uint64_t NextEventTime() const;

        private:
            std::vector<ReplayEvent> _events;
            std::size_t _next = 0;
    };
}
