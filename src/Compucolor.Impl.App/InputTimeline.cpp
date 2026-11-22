#include <Compucolor/App/Replay/InputTimeline.h>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

Compucolor::Impl::App::Replay::InputTimeline::InputTimeline(std::vector<ReplayEvent> events):
    _events(std::move(events))
{
    std::stable_sort(
        _events.begin(),
        _events.end(),
        [](const ReplayEvent& left, const ReplayEvent& right) {
            return left.emulatedNanoseconds < right.emulatedNanoseconds;
        }
    );
}

void Compucolor::Impl::App::Replay::InputTimeline::ApplyDue(
    const uint64_t now,
    const std::function<void(const ReplayEvent&)>& apply
)
{
    if (!apply)
    {
        throw std::invalid_argument("InputTimeline requires an apply callback");
    }

    while (_next < _events.size() && _events[_next].emulatedNanoseconds <= now)
    {
        apply(_events[_next]);
        ++_next;
    }
}

bool Compucolor::Impl::App::Replay::InputTimeline::HasPending() const
{
    return _next < _events.size();
}

uint64_t Compucolor::Impl::App::Replay::InputTimeline::NextEventTime() const
{
    if (!HasPending())
    {
        return std::numeric_limits<uint64_t>::max();
    }

    return _events[_next].emulatedNanoseconds;
}
