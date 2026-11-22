#include <Compucolor/App/Replay/InputRecorder.h>

#include <algorithm>
#include <stdexcept>
#include <utility>

void Compucolor::Impl::App::Replay::InputRecorder::Start(
    const boost::json::value& state,
    const uint64_t startNanoseconds
)
{
    std::lock_guard<std::mutex> lock(_mutex);

    _state = state;
    _startNanoseconds = startNanoseconds;
    _lastNanoseconds = startNanoseconds;
    _events.clear();
    _recording = true;
}

void Compucolor::Impl::App::Replay::InputRecorder::Record(
    const bool down,
    const Compucolor::Keyboard::CompucolorIIKey key,
    const uint64_t nowNanoseconds
)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_recording)
    {
        return;
    }

    const uint64_t timestamp = std::max(nowNanoseconds, _lastNanoseconds);

    _events.push_back(ReplayEvent{timestamp, key, down});
    _lastNanoseconds = timestamp;
}

void Compucolor::Impl::App::Replay::InputRecorder::Stop()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _recording = false;
}

bool Compucolor::Impl::App::Replay::InputRecorder::IsRecording() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _recording;
}

Compucolor::Impl::App::Replay::ReplayBundle Compucolor::Impl::App::Replay::InputRecorder::BuildBundle(
    const uint64_t stopNanoseconds,
    std::string createdBy
) const
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_recording)
    {
        throw std::runtime_error("No replay recording is active");
    }

    // Zero-length sessions are valid in the GUI but bundles require stop > state.
    const uint64_t stop = std::max(stopNanoseconds, _startNanoseconds + 1);

    return ReplayBundle(
        _state,
        _events,
        stop,
        std::move(createdBy)
    );
}
