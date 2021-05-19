#pragma once

#include <Compucolor/Logger/ILogger.h>

class NoopLogger: public ILogger
{
    virtual void Log(LogLevel level, std::string message, ...) override {};

    virtual void LogTrace(std::string message, ...) override {};
    virtual void LogDebug(std::string message, ...) override {};
    virtual void LogInformation(std::string message, ...) override {};
    virtual void LogWarning(std::string message, ...) override {};
    virtual void LogError(std::string message, ...) override {};
};