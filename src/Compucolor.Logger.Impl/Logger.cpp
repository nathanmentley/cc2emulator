#include <Compucolor.Logger.Impl/Logger.h>

Logger::Logger():
    _provider({}),
    _logLevel({})
{
}

void Logger::SetProvider(std::shared_ptr<ILoggerProvider> provider)
{
    _provider = provider;
}

void Logger::SetLogLevel(LogLevel level)
{
    _logLevel = level;
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
    if (_provider.has_value())
    {
        LogLevel currentLevel = _logLevel.value_or(LogLevel::Error);

        if (level >= currentLevel)
        {
            _provider.value()->Log(level, message, args);
        }
    }
}