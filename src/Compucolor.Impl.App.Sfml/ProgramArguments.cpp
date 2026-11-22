#include "ProgramArguments.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    void SetOnce(std::string& destination, const std::string& flag, const std::string& value)
    {
        if (value.empty())
        {
            throw std::runtime_error(flag + " requires a non-empty path");
        }

        if (!destination.empty())
        {
            throw std::runtime_error(flag + " was supplied more than once");
        }

        destination = value;
    }
}

Compucolor::Impl::App::Sfml::ProgramArguments
Compucolor::Impl::App::Sfml::ParseProgramArguments(const int argc, const char** argv)
{
    ProgramArguments arguments;
    std::vector<std::string> filterSpecs;

    for (int i = 1; i < argc; i++)
    {
        const std::string argument = argv[i];

        if (argument == "--trace-filter")
        {
            if (i + 1 >= argc)
            {
                throw std::runtime_error("--trace-filter requires a value");
            }

            filterSpecs.push_back(argv[++i]);
            continue;
        }

        constexpr const char* FilterPrefix = "--trace-filter=";

        if (argument.rfind(FilterPrefix, 0) == 0)
        {
            filterSpecs.push_back(argument.substr(std::string(FilterPrefix).size()));
            continue;
        }

        struct Option {
            const char* name;
            std::string ProgramArguments::*field;
        };

        const Option options[] = {
            {"--trace-jsonl", &ProgramArguments::traceJsonlPath},
            {"--load-state", &ProgramArguments::loadStatePath},
            {"--replay-bundle", &ProgramArguments::replayBundlePath},
            {"--record-replay", &ProgramArguments::recordReplayPath}
        };

        bool handled = false;

        for (const Option& option : options)
        {
            const std::string name(option.name);

            if (argument == name)
            {
                if (i + 1 >= argc)
                {
                    throw std::runtime_error(name + " requires a path");
                }

                SetOnce(arguments.*(option.field), name, argv[++i]);
                handled = true;
                break;
            }

            const std::string prefix = name + "=";

            if (argument.rfind(prefix, 0) == 0)
            {
                SetOnce(arguments.*(option.field), name, argument.substr(prefix.size()));
                handled = true;
                break;
            }
        }

        if (!handled)
        {
            throw std::runtime_error("Unknown emulator option: " + argument);
        }
    }

    if (!arguments.loadStatePath.empty() && !arguments.replayBundlePath.empty())
    {
        throw std::runtime_error("--load-state and --replay-bundle are mutually exclusive");
    }

    if (!arguments.recordReplayPath.empty() && !arguments.replayBundlePath.empty())
    {
        throw std::runtime_error("--record-replay and --replay-bundle are mutually exclusive");
    }

    arguments.traceFilters = TraceFilterConfig::Parse(filterSpecs);

    return arguments;
}
