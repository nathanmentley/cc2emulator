#pragma once

#include <string>

#include <Compucolor/App/TraceFilterConfig.h>

namespace Compucolor::Impl::App::Sfml
{
    struct ProgramArguments {
        std::string traceJsonlPath;
        std::string loadStatePath;
        std::string replayBundlePath;
        std::string recordReplayPath;
        TraceFilterConfig traceFilters;
    };

    ProgramArguments ParseProgramArguments(int argc, const char** argv);
}
