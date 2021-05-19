#include <Compucolor.Logger.Impl/Logger.h>

Compucolor::Logger::Impl::Logger::Logger(std::shared_ptr<ILoggerProvider> provider):
    _provider(provider)
{
}

void Compucolor::Logger::Impl::Logger::Log(LogLevel level, std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(level, message, args);

    va_end(args);
}

void Compucolor::Logger::Impl::Logger::LogTrace(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(LogLevel::Trace, message, args);

    va_end(args);
}

void Compucolor::Logger::Impl::Logger::LogDebug(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(LogLevel::Debug, message, args);

    va_end(args);
}

void Compucolor::Logger::Impl::Logger::LogInformation(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(LogLevel::Information, message, args);

    va_end(args);
}

void Compucolor::Logger::Impl::Logger::LogWarning(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(LogLevel::Warning, message, args);

    va_end(args);
}

void Compucolor::Logger::Impl::Logger::LogError(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(LogLevel::Error, message, args);

    va_end(args);
}

void Compucolor::Logger::Impl::Logger::Log(LogLevel level, std::string message, va_list args)
{
    LogLevel currentLevel = _provider->GetLogLevel();

    if (level >= currentLevel)
    {
        va_list loggerArgs;
        va_copy(loggerArgs, args);

        _provider->Write(level, message, loggerArgs);
    }
}