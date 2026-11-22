#pragma once

#include <cstdint>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

namespace Compucolor::Impl::App::Replay
{
    // One emulated key transition. Time is absolute emulated nanoseconds from
    // the machine's own clock, not a host timestamp.
    struct ReplayEvent {
        uint64_t emulatedNanoseconds = 0;
        Compucolor::Keyboard::CompucolorIIKey key{};
        bool down = false;
    };
}
