#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace Compucolor::Impl::App::Sfml
{
    // Turns timestamped 1-bit edges into audio samples through the SoundWare RC filter.
    // PushEdge and Advance belong to the emulator thread; Render belongs to the audio thread.
    class SoundSynth {
        public:
            // Latency is how far audio trails emulation; a longer lag than maxLag is skipped.
            explicit SoundSynth(
                unsigned int sampleRate,
                uint64_t latencyNanoseconds = 40'000'000,
                uint64_t maxLagNanoseconds = 200'000'000
            );

            // Edges must arrive in time order. They are dropped if the queue is full.
            void PushEdge(uint64_t timeNanoseconds, bool high);

            // Emulation has reached this time, so every earlier edge has been pushed.
            void Advance(uint64_t emulatedNanoseconds);

            void SetVolume(float volume);

            // When the emulator has not produced enough time yet, the rest of the block repeats the last sample.
            void Render(float* output, std::size_t count);

        private:
            struct Edge {
                uint64_t timeNanoseconds;
                bool high;
            };

            static constexpr std::size_t QueueSize = 8192;

            // First-order RC low-pass: 1 kOhm and 0.04 uF, about 4 kHz.
            static constexpr double TauNanoseconds = 40'000.0;

            std::array<Edge, QueueSize> _queue{};
            std::atomic<std::size_t> _head{0};
            std::atomic<std::size_t> _tail{0};
            std::atomic<uint64_t> _emulatedNow{0};
            std::atomic<float> _volume{1.0f};

            const double _stepNanoseconds;
            const uint64_t _latencyNanoseconds;
            const uint64_t _maxLagNanoseconds;

            bool _started = false;
            double _timelineNanoseconds = 0.0;
            double _filterTimeNanoseconds = 0.0;
            double _filter = 0.0;
            bool _level = false;
            float _lastSample;

            void Resync(uint64_t emulatedNow);
            void StepFilter(double timeNanoseconds);
            [[nodiscard]] float Scale(double filtered) const;
    };
}
