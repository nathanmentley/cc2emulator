#pragma once

#include <optional>

#include <Compucolor.Logger/ILogger.h>
#include <Compucolor.Logger/ILoggerProvider.h>

class Logger: public ILogger {
    public:
        Logger();

        virtual void SetProvider(std::shared_ptr<ILoggerProvider> provider) override;

        virtual void SetLogLevel(LogLevel level) override;

        virtual void Log(LogLevel level, std::string message, ...) override;

        virtual void LogTrace(std::string message, ...) override;
        virtual void LogDebug(std::string message, ...) override;
        virtual void LogInformation(std::string message, ...) override;
        virtual void LogWarning(std::string message, ...) override;
        virtual void LogError(std::string message, ...) override;

    private:
        std::optional<std::shared_ptr<ILoggerProvider>> _provider;
        std::optional<LogLevel> _logLevel;

        void Log(LogLevel level, std::string message, va_list args);
};