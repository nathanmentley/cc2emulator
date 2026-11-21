#include "AudioOutput.h"

#include <iostream>

#include <algorithm>

#include <cmath>
#include <cstdint>

using Compucolor::Impl::App::Sfml::AudioOutput;

AudioOutput::AudioOutput(const unsigned int sampleRate):
    _synth(sampleRate),
    _floats(ChunkSamples),
    _samples(ChunkSamples)
{
    initialize(1, sampleRate, {sf::SoundChannel::Mono});
}

AudioOutput::~AudioOutput()
{
    stop();
}

void AudioOutput::SetLevel(const bool high, const uint64_t emulatedNanoseconds)
{
    if (_enabled.load(std::memory_order_relaxed))
    {
        _synth.PushEdge(emulatedNanoseconds, high);
    }
}

void AudioOutput::Advance(const uint64_t emulatedNanoseconds)
{
    if (_enabled.load(std::memory_order_relaxed))
    {
        _synth.Advance(emulatedNanoseconds);
    }
}

void AudioOutput::SetEnabled(const bool enabled)
{
    if (_enabled.exchange(enabled) == enabled)
    {
        return;
    }

    if (enabled) {
        play();
    } else {
        stop();
    }
}

bool AudioOutput::IsEnabled() const
{
    return _enabled.load(std::memory_order_relaxed);
}

void AudioOutput::SetVolume(const float volume)
{
    _synth.SetVolume(volume);
}

static unsigned int calls = 0;
bool AudioOutput::onGetData(Chunk& data)
{

    if (++calls % 100 == 0)
    {
        const auto [min, max] = std::minmax_element(
            _floats.begin(),
            _floats.end()
        );
    }

    _synth.Render(_floats.data(), _floats.size());

    const auto [minIt, maxIt] = std::minmax_element(
        _samples.begin(),
        _samples.end()
    );

    for (std::size_t i = 0; i < _floats.size(); i++)
    {
        _samples[i] = static_cast<std::int16_t>(std::clamp(_floats[i], -1.0f, 1.0f) * 32767.0f);
    }

    data.samples = _samples.data();
    data.sampleCount = _samples.size();

    return true;
}

void AudioOutput::onSeek([[maybe_unused]] const sf::Time timeOffset)
{
}
