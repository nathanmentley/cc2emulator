#pragma once

#include <cstdio>
#include <execinfo.h>
#include <csignal>
#include <cstdlib>
#include <unistd.h>

namespace Compucolor::Impl::App::Sfml
{
    void ErrorHandler(int sig);
}