#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>

#include "ErrorHandler.h"
#include "Program.h"
#include "ProgramArguments.h"

using Compucolor::Impl::App::Sfml::ErrorHandler;
using Compucolor::Impl::App::Sfml::ParseProgramArguments;
using Compucolor::Impl::App::Sfml::Program;

int main([[maybe_unused]] const int argc, [[maybe_unused]] const char** argv) {
    signal(SIGSEGV, ErrorHandler);

    try
    {
        return Program(ParseProgramArguments(argc, argv)).Run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';

        return EXIT_FAILURE;
    }
}
