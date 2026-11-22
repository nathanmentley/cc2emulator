#include <Compucolor/App/Replay/ReplayRunner.h>

#include <stdexcept>
#include <utility>

#include <Compucolor/App/Replay/InputTimeline.h>

Compucolor::Impl::App::Replay::ReplayRunner::ReplayRunner(
    std::shared_ptr<Compucolor::ICompucolorEmulator> emulator,
    std::shared_ptr<ManualScheduler> scheduler,
    std::shared_ptr<Compucolor::Impl::App::DebugHub> debugHub
):
    _emulator(std::move(emulator)),
    _scheduler(std::move(scheduler)),
    _debugHub(std::move(debugHub))
{
    if (!_emulator || !_scheduler || !_debugHub)
    {
        throw std::invalid_argument("ReplayRunner requires emulator, scheduler and debug hub");
    }
}

void Compucolor::Impl::App::Replay::ReplayRunner::Run(const ReplayBundle& bundle)
{
    InputTimeline timeline(bundle.Input());

    const auto applyEvent = [this](const ReplayEvent& event) {
        if (event.down)
        {
            _emulator->OnKeyDown(event.key);
        }
        else
        {
            _emulator->OnKeyUp(event.key);
        }
    };

    _debugHub->SetEnabled(false);
    _emulator->SetDebugSink(_debugHub);

    bool started = false;

    try
    {
        _emulator->Start();
        started = true;

        // Start() loads the ROM and starts devices. The saved state must be
        // restored afterwards so its memory image wins over the ROM load.
        _emulator->LoadState(
            std::make_shared<boost::json::value>(bundle.State())
        );

        // Saved states may contain wall-clock pacing. Replay must never use it.
        _emulator->SetSpeedRegulated(false);

        // Recording begins only after the state is loaded, so startup work
        // cannot leak into the golden trace.
        _debugHub->SetEnabled(true);

        uint64_t current = _emulator->GetEmulatedNanoseconds();

        while (current < bundle.StopNanoseconds())
        {
            timeline.ApplyDue(current, applyEvent);

            const uint64_t before = current;
            _scheduler->Advance(ManualScheduler::Interval());
            current = _emulator->GetEmulatedNanoseconds();

            if (current <= before)
            {
                throw std::runtime_error("Replay did not advance emulated time");
            }
        }

        // Tick-boundary injection can overshoot an event time. Apply anything
        // still due at the stop boundary so the final keyboard state is correct.
        timeline.ApplyDue(bundle.StopNanoseconds(), applyEvent);

        auto state = _emulator->SaveState();

        if (!state)
        {
            throw std::runtime_error("Emulator returned an empty final state");
        }

        _finalState = *state;
    }
    catch (...)
    {
        if (started)
        {
            _emulator->Stop();
        }

        throw;
    }

    _emulator->Stop();
}

const boost::json::value& Compucolor::Impl::App::Replay::ReplayRunner::FinalState() const
{
    return _finalState;
}

uint64_t Compucolor::Impl::App::Replay::ReplayRunner::FinalEmulatedNanoseconds() const
{
    return _emulator->GetEmulatedNanoseconds();
}
