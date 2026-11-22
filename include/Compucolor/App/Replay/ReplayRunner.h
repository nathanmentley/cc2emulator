#pragma once

#include <cstdint>
#include <memory>

#include <boost/json.hpp>

#include <Compucolor/ICompucolorEmulator.h>

#include <Compucolor/App/DebugHub.h>
#include <Compucolor/App/Replay/ManualScheduler.h>
#include <Compucolor/App/Replay/ReplayBundle.h>

namespace Compucolor::Impl::App::Replay
{
    // Drives one replay bundle to completion on a manual scheduler. This class
    // is the deterministic oracle used by the headless CLI and golden tests.
    class ReplayRunner {
        public:
            ReplayRunner(
                std::shared_ptr<Compucolor::ICompucolorEmulator> emulator,
                std::shared_ptr<ManualScheduler> scheduler,
                std::shared_ptr<Compucolor::Impl::App::DebugHub> debugHub
            );

            void Run(const ReplayBundle& bundle);

            [[nodiscard]] const boost::json::value& FinalState() const;
            [[nodiscard]] uint64_t FinalEmulatedNanoseconds() const;

        private:
            std::shared_ptr<Compucolor::ICompucolorEmulator> _emulator;
            std::shared_ptr<ManualScheduler> _scheduler;
            std::shared_ptr<Compucolor::Impl::App::DebugHub> _debugHub;
            boost::json::value _finalState{};
    };
}
