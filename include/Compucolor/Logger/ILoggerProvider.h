#pragma once

#include <string>

#include <Compucolor/Logger/LogLevel.h>

namespace Compucolor::Logger
{
    class ILoggerProvider {
        public:
            virtual ~ILoggerProvider() = default;
    
            virtual void Write(LogLevel logLevel, std::string message, va_list args) = 0;
    
            virtual void SetLogLevel(LogLevel level) = 0;
    
            virtual LogLevel GetLogLevel() = 0;
    };
}