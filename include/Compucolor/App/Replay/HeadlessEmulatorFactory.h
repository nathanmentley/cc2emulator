#pragma once

#include <memory>

#include <Compucolor/ICompucolorEmulator.h>

#include <Compucolor/App/Replay/ManualScheduler.h>

namespace Compucolor::Impl::App::Replay
{
    struct HeadlessMachine {
        std::shared_ptr<Compucolor::ICompucolorEmulator> emulator;
        std::shared_ptr<ManualScheduler> scheduler;
    };

    // Builds the real emulator components with no host GUI, no audio device and
    // no threaded scheduler. Both the machine tick and CRT refresh are driven
    // by the returned ManualScheduler.
    HeadlessMachine CreateHeadlessMachine();
}
