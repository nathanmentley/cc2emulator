#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include "ErrorHandler.h"
#include "Program.h"

using Compucolor::Impl::App::Sfml::ErrorHandler;
using Compucolor::Impl::App::Sfml::Program;

namespace
{
    std::string TracePathFromArguments(const int argc, const char** argv)
    {
        for (int i = 1; i < argc; i++)
        {
            const std::string argument = argv[i];

            if (argument == "--trace-jsonl")
            {
                if (i + 1 >= argc)
                {
                    throw std::runtime_error("--trace-jsonl requires a path");
                }

                return argv[++i];
            }

            constexpr const char* Prefix = "--trace-jsonl=";
            if (argument.rfind(Prefix, 0) == 0)
            {
                const std::string path = argument.substr(std::string(Prefix).size());

                if (path.empty())
                {
                    throw std::runtime_error("--trace-jsonl requires a path");
                }

                return path;
            }
        }

        return {};
    }
}

int main([[maybe_unused]] const int argc, [[maybe_unused]] const char** argv) {
    signal(SIGSEGV, ErrorHandler);

    try
    {
        return Program(TracePathFromArguments(argc, argv)).Run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';

        return EXIT_FAILURE;
    }
}
