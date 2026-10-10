#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

#include <SFML/Audio.hpp>

#include <Compucolor/Common/IAudioOutput.h>

#include "SoundSynth.h"

namespace Compucolor::Impl::App::Sfml
{
    // Plays the SoundWare signal through the sound card; stays silent and idle until enabled.
    class AudioOutput: public sf::SoundStream, public Compucolor::Common::IAudioOutput {
        public:
            explicit AudioOutput(unsigned int sampleRate = 44100);
            ~AudioOutput() override;

            void SetLevel(bool high, uint64_t emulatedNanoseconds) override;
            void Advance(uint64_t emulatedNanoseconds) override;

            void SetEnabled(bool enabled);
            [[nodiscard]] bool IsEnabled() const;

            // 0.0 to 1.0
            void SetVolume(float volume);

        private:
            static constexpr std::size_t ChunkSamples = 1024;

            SoundSynth _synth;
            std::atomic<bool> _enabled{false};
            std::vector<float> _floats;
            std::vector<std::int16_t> _samples;

            bool onGetData(Chunk& data) override;
            void onSeek(sf::Time timeOffset) override;
    };
}
