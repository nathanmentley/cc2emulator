#include "SoundSynth.h"

#include <algorithm>
#include <cmath>

using Compucolor::Impl::App::Sfml::SoundSynth;

SoundSynth::SoundSynth(
    const unsigned int sampleRate,
    const uint64_t latencyNanoseconds,
    const uint64_t maxLagNanoseconds
):
    _stepNanoseconds(1e9 / static_cast<double>(sampleRate)),
    _latencyNanoseconds(latencyNanoseconds),
    _maxLagNanoseconds(maxLagNanoseconds),
    _lastSample(Scale(0.0))
{
}

void SoundSynth::PushEdge(const uint64_t timeNanoseconds, const bool high)
{
    const std::size_t head = _head.load(std::memory_order_relaxed);
    const std::size_t next = (head + 1) % QueueSize;

    if (next == _tail.load(std::memory_order_acquire))
    {
        return;
    }

    _queue[head] = Edge{timeNanoseconds, high};
    _head.store(next, std::memory_order_release);
}

void SoundSynth::Advance(const uint64_t emulatedNanoseconds)
{
    _emulatedNow.store(emulatedNanoseconds, std::memory_order_release);
}

void SoundSynth::SetVolume(const float volume)
{
    _volume.store(std::clamp(volume, 0.0f, 1.0f), std::memory_order_relaxed);
}

float SoundSynth::Scale(const double filtered) const
{
    // Centre the 0..1 signal on zero and keep it quiet, as ccemu does.
    return static_cast<float>((0.5 * filtered - 0.25) * _volume.load(std::memory_order_relaxed));
}

void SoundSynth::StepFilter(const double timeNanoseconds)
{
    const double elapsed = std::max(0.0, timeNanoseconds - _filterTimeNanoseconds);
    const double target = _level ? 1.0 : 0.0;

    _filter = target + (_filter - target) * std::exp(-elapsed / TauNanoseconds);
    _filterTimeNanoseconds = std::max(timeNanoseconds, _filterTimeNanoseconds);
}

void SoundSynth::Resync(const uint64_t emulatedNow)
{
    const double target = static_cast<double>(emulatedNow - _latencyNanoseconds);

    // Skipped edges are not played, but the last one decides the level we continue from.
    for (;;)
    {
        const std::size_t tail = _tail.load(std::memory_order_relaxed);

        if (tail == _head.load(std::memory_order_acquire) || static_cast<double>(_queue[tail].timeNanoseconds) > target)
        {
            break;
        }

        _level = _queue[tail].high;
        _tail.store((tail + 1) % QueueSize, std::memory_order_release);
    }

    _timelineNanoseconds = target;
    _filterTimeNanoseconds = target;
    _started = true;
}

void SoundSynth::Render(float* const output, const std::size_t count)
{
    // Read the clock before the queue so every edge up to it is already visible.
    const uint64_t now = _emulatedNow.load(std::memory_order_acquire);

    if (!_started)
    {
        if (now < _latencyNanoseconds)
        {
            std::fill(output, output + count, _lastSample);
            return;
        }

        Resync(now);
    }
    else if (static_cast<double>(now) - _timelineNanoseconds > static_cast<double>(_maxLagNanoseconds))
    {
        Resync(now);
    }

    for (std::size_t i = 0; i < count; i++)
    {
        const double next = _timelineNanoseconds + _stepNanoseconds;

        if (next > static_cast<double>(now))
        {
            std::fill(output + i, output + count, _lastSample);
            return;
        }

        for (;;)
        {
            const std::size_t tail = _tail.load(std::memory_order_relaxed);

            if (tail == _head.load(std::memory_order_acquire) || static_cast<double>(_queue[tail].timeNanoseconds) > next)
            {
                break;
            }

            StepFilter(static_cast<double>(_queue[tail].timeNanoseconds));
            _level = _queue[tail].high;
            _tail.store((tail + 1) % QueueSize, std::memory_order_release);
        }

        StepFilter(next);
        _timelineNanoseconds = next;
        _lastSample = Scale(_filter);
        output[i] = _lastSample;
    }
}
