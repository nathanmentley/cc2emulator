#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <Compucolor/App/DebugHub.h>
#include <Compucolor/App/Replay/HeadlessEmulatorFactory.h>
#include <Compucolor/App/JsonlDebugSink.h>
#include <Compucolor/App/Replay/ReplayBundle.h>
#include <Compucolor/App/Replay/ReplayRunner.h>
#include <Compucolor/App/Replay/StateHash.h>
#include <Compucolor/App/TraceFilterConfig.h>

namespace
{
    struct Options {
        std::string bundlePath;
        std::string tracePath;
        std::string expectedHash;
        Compucolor::Impl::App::TraceFilterConfig filters;
    };

    void RequireValue(
        const int argc,
        const char** argv,
        int& index,
        const std::string& flag
    )
    {
        if (index + 1 >= argc)
        {
            throw std::runtime_error(flag + " requires a value");
        }

        ++index;
    }

    std::string FromEquals(const std::string& argument, const std::string& prefix)
    {
        const std::string value = argument.substr(prefix.size());

        if (value.empty())
        {
            throw std::runtime_error(prefix.substr(0, prefix.size() - 1) + " requires a value");
        }

        return value;
    }

    Options ParseArguments(const int argc, const char** argv)
    {
        Options options;
        std::vector<std::string> filterSpecs;

        for (int i = 1; i < argc; i++)
        {
            const std::string argument = argv[i];

            if (argument == "--bundle")
            {
                RequireValue(argc, argv, i, argument);
                options.bundlePath = argv[i];
                continue;
            }

            if (argument.rfind("--bundle=", 0) == 0)
            {
                options.bundlePath = FromEquals(argument, "--bundle=");
                continue;
            }

            if (argument == "--trace-jsonl")
            {
                RequireValue(argc, argv, i, argument);
                options.tracePath = argv[i];
                continue;
            }

            if (argument.rfind("--trace-jsonl=", 0) == 0)
            {
                options.tracePath = FromEquals(argument, "--trace-jsonl=");
                continue;
            }

            if (argument == "--trace-filter")
            {
                RequireValue(argc, argv, i, argument);
                filterSpecs.push_back(argv[i]);
                continue;
            }

            if (argument.rfind("--trace-filter=", 0) == 0)
            {
                filterSpecs.push_back(FromEquals(argument, "--trace-filter="));
                continue;
            }

            if (argument == "--expected-state-hash")
            {
                RequireValue(argc, argv, i, argument);
                options.expectedHash = argv[i];
                continue;
            }

            if (argument.rfind("--expected-state-hash=", 0) == 0)
            {
                options.expectedHash = FromEquals(argument, "--expected-state-hash=");
                continue;
            }

            if (argument == "--help")
            {
                std::cout
                    << "Usage: Compucolor.Impl.App.Headless --bundle <path> [options]\n"
                    << "\n"
                    << "Options:\n"
                    << "  --trace-jsonl <path>          Write replay trace as JSONL\n"
                    << "  --trace-filter <spec>        Filter JSONL records (repeatable)\n"
                    << "  --expected-state-hash <hex>  Fail if the final SHA-256 differs\n";
                std::exit(EXIT_SUCCESS);
            }

            throw std::runtime_error("Unknown replay option: " + argument);
        }

        if (options.bundlePath.empty())
        {
            throw std::runtime_error("--bundle <path> is required");
        }

        options.filters = Compucolor::Impl::App::TraceFilterConfig::Parse(
            filterSpecs
        );

        return options;
    }

    std::string LowerCopy(std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](const unsigned char character) {
                return static_cast<char>(std::tolower(character));
            }
        );

        return value;
    }
}

int main(const int argc, const char** argv)
{
    try
    {
        const Options options = ParseArguments(argc, argv);
        const auto bundle = Compucolor::Impl::App::Replay::ReplayBundle::ParseFile(
            options.bundlePath
        );

        auto hub = std::make_shared<
            Compucolor::Impl::App::DebugHub
        >();

        std::shared_ptr<Compucolor::Impl::App::JsonlDebugSink> sink;

        if (!options.tracePath.empty())
        {
            sink = std::make_shared<Compucolor::Impl::App::JsonlDebugSink>(
                options.tracePath,
                options.filters
            );
            hub->AddSink(sink);
        }

        const auto machine = Compucolor::Impl::App::Replay::CreateHeadlessMachine();
        Compucolor::Impl::App::Replay::ReplayRunner runner(
            machine.emulator,
            machine.scheduler,
            hub
        );

        runner.Run(bundle);

        if (sink)
        {
            sink->Flush();
        }

        const std::string hash =
            Compucolor::Impl::App::Replay::StateHash::OfState(runner.FinalState());

        if (!options.expectedHash.empty() &&
            LowerCopy(options.expectedHash) != LowerCopy(hash))
        {
            throw std::runtime_error(
                "Final state hash mismatch: expected " + options.expectedHash +
                ", got " + hash
            );
        }

        std::cout << "stateHash: " << hash << '\n';
        std::cout << "emulatedNanoseconds: "
                  << runner.FinalEmulatedNanoseconds() << '\n';

        return EXIT_SUCCESS;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
