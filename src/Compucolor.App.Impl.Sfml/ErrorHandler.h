#pragma once

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

namespace Compucolor::App::Impl::Smfl
{
    void ErrorHandler(int sig);
}