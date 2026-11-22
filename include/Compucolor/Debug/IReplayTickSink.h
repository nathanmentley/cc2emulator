#pragma once

#include <cstdint>

namespace Compucolor::Debug
{
    // Optional interface for host-side consumers that need the exact start of
    // a machine tick. It is deliberately separate from IDebugSink so adding the
    // tick boundary does not change that interface's virtual table.
    class IReplayTickSink {
        public:
            virtual ~IReplayTickSink() = default;

            virtual void OnTickBoundary(uint64_t emulatedNanoseconds) = 0;
    };
}
