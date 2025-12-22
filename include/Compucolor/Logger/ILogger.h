#pragma once

#include <string>

#include <Compucolor/Common/IPlugin.h>

#include <Compucolor/Logger/LogLevel.h>

namespace Compucolor::Logger
{
    class ILogger: public Common::IPlugin {
        public:
            virtual ~ILogger() = default;
    
            virtual void Log(LogLevel level, std::string message, ...) = 0;
    
            virtual void LogTrace(std::string message, ...) = 0;
            virtual void LogDebug(std::string message, ...) = 0;
            virtual void LogInformation(std::string message, ...) = 0;
            virtual void LogWarning(std::string message, ...) = 0;
            virtual void LogError(std::string message, ...) = 0;
    };
}