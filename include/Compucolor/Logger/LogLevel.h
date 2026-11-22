#pragma once

namespace Compucolor::Logger
{
    // Ordered severity levels; providers emit messages at or above their configured level.
    enum class LogLevel
    {
        Trace = 1,
        Debug = 2,
        Information = 3,
        Warning = 4,
        Error = 5
    };
}
