#include <Compucolor/App/Replay/ReplayInputSink.h>

#include <stdexcept>
#include <utility>

void Compucolor::Impl::App::Replay::ReplayInputSink::Start(
    const ReplayBundle& bundle,
    ApplyHandler apply,
    const uint16_t initialPc
)
{
    if (!apply)
    {
        throw std::invalid_argument("ReplayInputSink requires an apply handler");
    }

    std::lock_guard<std::mutex> lock(_mutex);

    const auto& stateTime = bundle.State().as_object().at("emulatedNanoseconds");

    // boost.json parses positive JSON integers as int64, while an in-memory
    // snapshot from the processor may already be uint64. Accept both.
    const uint64_t startNanoseconds = stateTime.is_uint64()
        ? stateTime.as_uint64()
        : static_cast<uint64_t>(stateTime.as_int64());

    _timeline = std::make_unique<InputTimeline>(bundle.Input());
    _apply = std::move(apply);
    _stopNanoseconds = bundle.StopNanoseconds();
    _currentNanoseconds = startNanoseconds;
    _appliedEvents = 0;
    _totalEvents = bundle.Input().size();
    _pc = initialPc;
}

void Compucolor::Impl::App::Replay::ReplayInputSink::Stop()
{
    std::lock_guard<std::mutex> lock(_mutex);

    _timeline.reset();
    _apply = nullptr;
}

bool Compucolor::Impl::App::Replay::ReplayInputSink::IsActive() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _timeline != nullptr;
}

Compucolor::Impl::App::Replay::ReplayProgress Compucolor::Impl::App::Replay::ReplayInputSink::GetProgress() const
{
    std::lock_guard<std::mutex> lock(_mutex);

    ReplayProgress progress;
    progress.active = _timeline != nullptr;
    progress.currentNanoseconds = _currentNanoseconds;
    progress.stopNanoseconds = _stopNanoseconds;
    progress.appliedEvents = _appliedEvents;
    progress.totalEvents = _totalEvents;

    if (_timeline)
    {
        progress.nextEventNanoseconds = _timeline->NextEventTime();
    }

    return progress;
}

void Compucolor::Impl::App::Replay::ReplayInputSink::SetCpuState(
    const Compucolor::Debug::CpuState& state
)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _currentNanoseconds = state.emulatedNanoseconds;
}

void Compucolor::Impl::App::Replay::ReplayInputSink::OnTickBoundary(
    const uint64_t emulatedNanoseconds
)
{
    std::lock_guard<std::mutex> lock(_mutex);

    _currentNanoseconds = emulatedNanoseconds;

    if (!_timeline || !_apply)
    {
        return;
    }

    _timeline->ApplyDue(
        _currentNanoseconds,
        [this](const ReplayEvent& event) {
            _apply(event);
            ++_appliedEvents;
        }
    );

    if (_currentNanoseconds >= _stopNanoseconds)
    {
        _timeline.reset();
        _apply = nullptr;
    }
}

void Compucolor::Impl::App::Replay::ReplayInputSink::SetCurrentPc(const uint16_t pc)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _pc = pc;
}

uint16_t Compucolor::Impl::App::Replay::ReplayInputSink::GetCurrentPc() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _pc;
}

void Compucolor::Impl::App::Replay::ReplayInputSink::OnInstruction(
    const Compucolor::Debug::InstructionEvent&
)
{
}

void Compucolor::Impl::App::Replay::ReplayInputSink::OnPort(const Compucolor::Debug::PortEvent&)
{
}

void Compucolor::Impl::App::Replay::ReplayInputSink::SetTms5501State(
    const Compucolor::Debug::Tms5501State&
)
{
}

void Compucolor::Impl::App::Replay::ReplayInputSink::SetSmc5027State(
    const Compucolor::Debug::Smc5027State&
)
{
}
