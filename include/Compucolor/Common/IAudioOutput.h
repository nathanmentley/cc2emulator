#pragma once

#include <cstdint>

namespace Compucolor::Common
{
    // Receives the SoundWare add-on's 1-bit signal, stamped in emulated time.
    class IAudioOutput {
        public:
            virtual ~IAudioOutput() = default;

            // The TXD line changed; emulatedNanoseconds is when the CPU executed the write.
            virtual void SetLevel(bool high, uint64_t emulatedNanoseconds) = 0;

            // Emulation has reached this time, so no earlier edge is still to come.
            virtual void Advance(uint64_t emulatedNanoseconds) = 0;
        };
}
