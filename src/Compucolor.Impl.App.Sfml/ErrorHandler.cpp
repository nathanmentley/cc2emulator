#include "ErrorHandler.h"

void Compucolor::Impl::App::Sfml::ErrorHandler(int sig) {
    void *array[10];

    // get void*'s for all entries on the stack
    int size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}