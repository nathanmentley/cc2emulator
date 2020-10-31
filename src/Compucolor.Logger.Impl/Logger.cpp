#include <Compucolor.Logger.Impl/Logger.h>

Logger::Logger():
    _providers(std::vector<std::shared_ptr<ILoggerProvider>>())
{
}

void Logger::AddProvider(std::shared_ptr<ILoggerProvider> provider)
{
    _providers.push_back(provider);
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
    std::vector<std::shared_ptr<ILoggerProvider>>::iterator provider;

    for(provider = _providers.begin(); provider != _providers.end(); provider++)
    {
        LogLevel currentLevel = (*provider)->GetLogLevel();

        if (level >= currentLevel)
        {
            va_list loggerArgs;
            va_copy(loggerArgs, args);

            (*provider)->Write(level, message, loggerArgs);
        }
    }
}