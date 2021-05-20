#pragma once

#include <memory>
#include <string>

#include <Compucolor/Logger/ILoggerProvider.h>
#include <Compucolor/Logger/LogLevel.h>

namespace Compucolor::Logger
{
    class ILogger {
        public:
            virtual ~ILogger() {}
    
            virtual void Log(LogLevel level, std::string message, ...) = 0;
    
            virtual void LogTrace(std::string message, ...) = 0;
            virtual void LogDebug(std::string message, ...) = 0;
            virtual void LogInformation(std::string message, ...) = 0;
            virtual void LogWarning(std::string message, ...) = 0;
            virtual void LogError(std::string message, ...) = 0;
    };
}