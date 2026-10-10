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
        EXPECT_FLOAT_EQ(Low, sample);
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
        EXPECT_FLOAT_EQ(Low, sample);
    }
}

TEST(SoundSynth, EdgeIsHeardAtItsTimestamp) {
    //Arrange: a rising edge halfway through a 10 ms window
    SoundSynth synth(SampleRate, Latency);
    synth.PushEdge(Second + 5'000'000, true);
    synth.Advance(Second + Latency);

    //Act
    const auto samples = Render(synth, TenMilliseconds);

    //Assert
    EXPECT_NEAR(Low, samples[100], 0.001f);
    EXPECT_NEAR(High, samples[400], 0.001f);
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
    SoundSynth synth(SampleRate, Latency);
    synth.SetVolume(0.5f);
    synth.PushEdge(Second, true);
    synth.Advance(Second + Latency);

    //Act
    const auto samples = Render(synth, TenMilliseconds);

    //Assert
    EXPECT_NEAR(High * 0.5f, samples.back(), 0.001f);
}

TEST(SoundSynth, VolumeIsClampedToOne) {
    //Arrange
    SoundSynth synth(SampleRate, Latency);
    synth.SetVolume(5.0f);
    synth.Advance(Second);

    //Act
    const auto samples = Render(synth, 16);

    //Assert
    EXPECT_FLOAT_EQ(Low, samples[0]);
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
    SoundSynth synth(SampleRate, Latency);
    synth.Advance(Second);
    Render(synth, TenMilliseconds);

    //Act: emulation races ahead by half a minute and the line goes high meanwhile
    synth.PushEdge(30 * Second - 500'000'000, true);
    synth.Advance(30 * Second);
    const auto samples = Render(synth, TenMilliseconds);

    //Assert: audio jumped to the present, so the output already reflects the high level
    EXPECT_NEAR(High, samples.back(), 0.001f);
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

    //Assert
    for (const float sample : samples)
    {
        EXPECT_TRUE(std::isfinite(sample));
        EXPECT_LE(std::abs(sample), 0.26f);
    }
}
