#pragma once

#include <vector>

#include <Compucolor.Logger/ILogger.h>
#include <Compucolor.Logger/ILoggerProvider.h>

namespace Compucolor::Logger::Impl
{
    class Logger: public ILogger {
        public:
            Logger(std::shared_ptr<ILoggerProvider> provider);
    
            virtual void Log(LogLevel level, std::string message, ...) override;
    
            virtual void LogTrace(std::string message, ...) override;
            virtual void LogDebug(std::string message, ...) override;
            virtual void LogInformation(std::string message, ...) override;
            virtual void LogWarning(std::string message, ...) override;
            virtual void LogError(std::string message, ...) override;
    
        private:
            std::shared_ptr<ILoggerProvider> _provider;
    
            void Log(LogLevel level, std::string message, va_list args);
    };
}