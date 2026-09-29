#include "ErrorHandler.h"
#include "Program.h"

using Compucolor::Impl::App::Sfml::ErrorHandler;
using Compucolor::Impl::App::Sfml::Program;

int main([[maybe_unused]] const int argc, [[maybe_unused]] const char** argv) {
    signal(SIGSEGV, ErrorHandler);

    return Program().Run();
}