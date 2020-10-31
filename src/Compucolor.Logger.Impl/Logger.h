#pragma once

#include <vector>

#include <Compucolor.Logger/ILogger.h>
#include <Compucolor.Logger/ILoggerProvider.h>

class Logger: public ILogger {
    public:
        Logger();

        virtual void AddProvider(std::shared_ptr<ILoggerProvider> provider) override;

        virtual void Log(LogLevel level, std::string message, ...) override;

        virtual void LogTrace(std::string message, ...) override;
        virtual void LogDebug(std::string message, ...) override;
        virtual void LogInformation(std::string message, ...) override;
        virtual void LogWarning(std::string message, ...) override;
        virtual void LogError(std::string message, ...) override;

    private:
        std::vector<std::shared_ptr<ILoggerProvider>> _providers;

        void Log(LogLevel level, std::string message, va_list args);
};