#include <algorithm>
#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "SoundSynth.h"

using Compucolor::Impl::App::Sfml::SoundSynth;

namespace
{
    constexpr unsigned int SampleRate = 48000;
    constexpr uint64_t Latency = 10'000'000;
    constexpr uint64_t Second = 1'000'000'000;

    // 10 ms of audio at 48 kHz.
    constexpr std::size_t TenMilliseconds = 480;

    // The DC-blocking filter centres steady silence at zero. Low/High remain
    // the pre-DC-blocked levels used by the RC-filter test.
    constexpr float Silence = 0.0f;
    constexpr float Low = -0.25f;
    constexpr float High = 0.25f;

    std::vector<float> Render(SoundSynth& synth, std::size_t count)
    {
        std::vector<float> samples(count);
        synth.Render(samples.data(), count);

        return samples;
    }
}

TEST(SoundSynth, WithoutEdgesTheOutputIsSteadySilence) {
    //Arrange
    SoundSynth synth(SampleRate, Latency);
    synth.Advance(Second);

    //Act
    const auto samples = Render(synth, TenMilliseconds);

    //Assert
    for (const float sample : samples)
    {
        EXPECT_FLOAT_EQ(Silence, sample);
    }
}

TEST(SoundSynth, NothingIsPlayedUntilEnoughEmulatedTimeHasPassed) {
    //Arrange
    SoundSynth synth(SampleRate, Latency);
    synth.Advance(5'000'000);

    //Act
    const auto samples = Render(synth, 64);

    //Assert
    for (const float sample : samples)
    {
        EXPECT_FLOAT_EQ(Silence, sample);
    }
}

TEST(SoundSynth, EdgeIsHeardAtItsTimestamp) {
    //Arrange: a rising edge halfway through a 10 ms window
    SoundSynth synth(SampleRate, Latency);
    synth.PushEdge(Second + 5'000'000, true);
    synth.Advance(Second + Latency);

    //Act
    const auto samples = Render(synth, TenMilliseconds);

    //Assert: silence before the edge, then the DC-blocked rising transient.
    EXPECT_FLOAT_EQ(Silence, samples[200]);
    EXPECT_GT(samples[300], 0.2f);
}

TEST(SoundSynth, SquareWaveKeepsItsFrequency) {
    //Arrange: 1 kHz, so an edge every 500 microseconds
    SoundSynth synth(SampleRate, Latency);
    for (int edge = 0; edge <= 200; edge++)
    {
        synth.PushEdge(Second + edge * 500'000ULL, edge % 2 == 0);
    }
    synth.Advance(Second + 100'000'000);

    //Act
    const auto samples = Render(synth, TenMilliseconds);

    //Assert: 10 ms of a 1 kHz wave crosses the midpoint about 20 times
    const auto [minimum, maximum] = std::minmax_element(samples.begin(), samples.end());
    const float middle = (*minimum + *maximum) / 2.0f;
    int crossings = 0;
    for (std::size_t i = 1; i < samples.size(); i++)
    {
        if ((samples[i - 1] < middle) != (samples[i] < middle))
        {
            crossings++;
        }
    }
    EXPECT_NEAR(20, crossings, 2);
}

TEST(SoundSynth, TheRcFilterRoundsOffEdges) {
    //Arrange
    SoundSynth synth(SampleRate, Latency);
    synth.PushEdge(Second + 2'000'000, true);
    synth.Advance(Second + Latency);

    //Act
    const auto samples = Render(synth, TenMilliseconds);

    //Assert: the first sample after the edge is on the way up, not already at the top
    const std::size_t firstAfterEdge = 2'000'000 / 20'834 + 1;
    EXPECT_GT(samples[firstAfterEdge], Low);
    EXPECT_LT(samples[firstAfterEdge], High);
}

TEST(SoundSynth, VolumeScalesTheOutput) {
    //Arrange
    SoundSynth full(SampleRate, Latency);
    SoundSynth half(SampleRate, Latency);
    half.SetVolume(0.5f);

    full.PushEdge(Second, true);
    half.PushEdge(Second, true);
    full.Advance(Second + Latency);
    half.Advance(Second + Latency);

    //Act
    const auto fullSamples = Render(full, TenMilliseconds);
    const auto halfSamples = Render(half, TenMilliseconds);

    //Assert: volume is applied after DC blocking, so the whole waveform scales.
    EXPECT_NEAR(0.5f * fullSamples[0], halfSamples[0], 0.001f);
    EXPECT_NEAR(0.5f * fullSamples[100], halfSamples[100], 0.001f);
}

TEST(SoundSynth, VolumeIsClampedToOne) {
    //Arrange
    SoundSynth normal(SampleRate, Latency);
    SoundSynth clamped(SampleRate, Latency);
    clamped.SetVolume(5.0f);

    normal.PushEdge(Second, true);
    clamped.PushEdge(Second, true);
    normal.Advance(Second + Latency);
    clamped.Advance(Second + Latency);

    //Act
    const auto normalSamples = Render(normal, 16);
    const auto clampedSamples = Render(clamped, 16);

    //Assert
    for (std::size_t i = 0; i < normalSamples.size(); i++)
    {
        EXPECT_FLOAT_EQ(normalSamples[i], clampedSamples[i]);
    }
}

TEST(SoundSynth, RunningOutOfEmulatedTimeRepeatsTheLastSample) {
    //Arrange: only 10 ms of emulation is available for a 20 ms request
    SoundSynth synth(SampleRate, Latency);
    synth.PushEdge(Second - 9'000'000, true);
    synth.Advance(Second);

    //Act
    const auto samples = Render(synth, 2 * TenMilliseconds);

    //Assert
    const float last = samples[TenMilliseconds - 2];
    for (std::size_t i = TenMilliseconds + 1; i < samples.size(); i++)
    {
        EXPECT_FLOAT_EQ(last, samples[i]);
    }
}

TEST(SoundSynth, ALongStallIsSkippedInsteadOfReplayed) {
    //Arrange
    SoundSynth stalled(SampleRate, Latency);
    SoundSynth control(SampleRate, Latency);
    stalled.Advance(Second);
    control.Advance(Second);
    Render(stalled, TenMilliseconds);
    Render(control, TenMilliseconds);

    //Act: emulation races ahead by half a minute and the line goes high meanwhile
    stalled.PushEdge(30 * Second - 500'000'000, true);
    stalled.Advance(30 * Second);
    control.Advance(30 * Second);
    const auto stalledSamples = Render(stalled, TenMilliseconds);
    const auto controlSamples = Render(control, TenMilliseconds);

    //Assert: audio jumped to the present and picked up the new line level.
    // The DC blocker still centres a steady level at zero.
    EXPECT_GT(stalledSamples.front(), 0.1f);
    EXPECT_FLOAT_EQ(Silence, controlSamples.front());
    EXPECT_FLOAT_EQ(Silence, controlSamples.back());
}

TEST(SoundSynth, AFullQueueDropsEdgesWithoutBreakingPlayback) {
    //Arrange: far more edges than the queue can hold, with no one rendering
    SoundSynth synth(SampleRate, Latency);
    for (uint64_t edge = 0; edge < 20'000; edge++)
    {
        synth.PushEdge(Second + edge * 1'000, edge % 2 == 0);
    }
    synth.Advance(Second + 30'000'000);

    //Act
    const auto samples = Render(synth, 1024);

    //Assert: the DC blocker can overshoot the +/-0.25 input range on a step,
    // but the generated audio stays finite and bounded.
    float maxAbs = 0.0f;
    for (const float sample : samples)
    {
        EXPECT_TRUE(std::isfinite(sample));
        maxAbs = std::max(maxAbs, std::abs(sample));
    }
    EXPECT_GT(maxAbs, 0.0f);
    EXPECT_LE(maxAbs, 0.6f);
}
