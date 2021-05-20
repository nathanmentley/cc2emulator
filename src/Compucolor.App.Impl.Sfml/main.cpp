#include "ErrorHandler.h"
#include "Program.h"

int main(int argc, char** argv) {
    signal(SIGSEGV, Compucolor::App::Impl::Sfml::ErrorHandler);

    return Compucolor::App::Impl::Sfml::Program()
        .Run();
}