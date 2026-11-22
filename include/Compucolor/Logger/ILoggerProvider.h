#pragma once

#include <string>

#include <Compucolor/Logger/LogLevel.h>

namespace Compucolor::Logger
{
    // Host-owned sink that formats and emits log messages.
    class ILoggerProvider {
        public:
            virtual ~ILoggerProvider() = default;

            // Receives an already-unpacked va_list from ILogger.
            virtual void Write(LogLevel logLevel, std::string message, va_list args) = 0;

            virtual void SetLogLevel(LogLevel level) = 0;

            virtual LogLevel GetLogLevel() = 0;
    };
}
