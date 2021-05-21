#include "ErrorHandler.h"
#include "Program.h"

int main(int argc, char** argv) {
    signal(SIGSEGV, Compucolor::Impl::App::Sfml::ErrorHandler);

    return Compucolor::Impl::App::Sfml::Program()
        .Run();
}