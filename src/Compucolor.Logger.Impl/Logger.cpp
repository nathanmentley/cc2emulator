#include <Compucolor.Logger.Impl/Logger.h>

Logger::Logger(std::shared_ptr<ILoggerProvider> provider):
    _provider(provider)
{
}

void Logger::Log(LogLevel level, std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(level, message, args);

    va_end(args);
}

void Logger::LogTrace(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(LogLevel::Trace, message, args);

    va_end(args);
}

void Logger::LogDebug(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(LogLevel::Debug, message, args);

    va_end(args);
}

void Logger::LogInformation(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(LogLevel::Information, message, args);

    va_end(args);
}

void Logger::LogWarning(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(LogLevel::Warning, message, args);

    va_end(args);
}

void Logger::LogError(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(LogLevel::Error, message, args);

    va_end(args);
}

void Logger::Log(LogLevel level, std::string message, va_list args)
{
    LogLevel currentLevel = _provider->GetLogLevel();

    if (level >= currentLevel)
    {
        va_list loggerArgs;
        va_copy(loggerArgs, args);

        _provider->Write(level, message, loggerArgs);
    }
}