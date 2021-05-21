#include "Logger.h"

Compucolor::Impl::Logger::Logger::Logger(std::shared_ptr<Compucolor::Logger::ILoggerProvider> provider):
    _provider(provider)
{
}

void Compucolor::Impl::Logger::Logger::Log(Compucolor::Logger::LogLevel level, std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(level, message, args);

    va_end(args);
}

void Compucolor::Impl::Logger::Logger::LogTrace(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(Compucolor::Logger::LogLevel::Trace, message, args);

    va_end(args);
}

void Compucolor::Impl::Logger::Logger::LogDebug(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(Compucolor::Logger::LogLevel::Debug, message, args);

    va_end(args);
}

void Compucolor::Impl::Logger::Logger::LogInformation(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(Compucolor::Logger::LogLevel::Information, message, args);

    va_end(args);
}

void Compucolor::Impl::Logger::Logger::LogWarning(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(Compucolor::Logger::LogLevel::Warning, message, args);

    va_end(args);
}

void Compucolor::Impl::Logger::Logger::LogError(std::string message, ...)
{
    va_list args;

    va_start(args, message);

    Log(Compucolor::Logger::LogLevel::Error, message, args);

    va_end(args);
}

void Compucolor::Impl::Logger::Logger::Log(Compucolor::Logger::LogLevel level, std::string message, va_list args)
{
    Compucolor::Logger::LogLevel currentLevel = _provider->GetLogLevel();

    if (level >= currentLevel)
    {
        va_list loggerArgs;
        va_copy(loggerArgs, args);

        _provider->Write(level, message, loggerArgs);
    }
}