#include "ConsoleLoggerProvider.h"

Compucolor::App::ConsoleLoggerProvider::ConsoleLoggerProvider(Logger::LogLevel logLevel):
    _logLevel(logLevel)
{
}

void Compucolor::App::ConsoleLoggerProvider::Write(Logger::LogLevel logLevel, std::string message, va_list args)
{
    Logger::LogLevel currentLevel = _logLevel
        .value_or(Logger::LogLevel::Error);

    if (logLevel < currentLevel)
    {
        return;
    }

    time_t timenow = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now()
    );

    printf(
        "%s[%s%s - %s%ld%s]%s ",
        ConsoleLoggerProvider::Blue.c_str(),
        GetLogLevel(logLevel).c_str(),
        ConsoleLoggerProvider::Blue.c_str(),
        ConsoleLoggerProvider::Green.c_str(),
        timenow,
        ConsoleLoggerProvider::Blue.c_str(),
        ConsoleLoggerProvider::Yellow.c_str()
    );

    vprintf(message.c_str(), args);
    printf("%s\n", ConsoleLoggerProvider::Reset.c_str());
}

void Compucolor::App::ConsoleLoggerProvider::SetLogLevel(Logger::LogLevel level)
{
    _logLevel = level;
}

Compucolor::Logger::LogLevel Compucolor::App::ConsoleLoggerProvider::GetLogLevel()
{
    return _logLevel.value_or(Logger::LogLevel::Error);
}

std::string Compucolor::App::ConsoleLoggerProvider::GetLogLevel(Logger::LogLevel logLevel)
{
    switch(logLevel)
    {
        case Logger::LogLevel::Trace:
            return ConsoleLoggerProvider::White + ConsoleLoggerProvider::LevelTraceName;
        case Logger::LogLevel::Debug:
            return ConsoleLoggerProvider::Cyan + ConsoleLoggerProvider::LevelDebugName;
        case Logger::LogLevel::Information:
            return ConsoleLoggerProvider::Magenta + ConsoleLoggerProvider::LevelInformationName;
        case Logger::LogLevel::Warning:
            return ConsoleLoggerProvider::Yellow + ConsoleLoggerProvider::LevelWarningName;
        case Logger::LogLevel::Error:
            return ConsoleLoggerProvider::Red + ConsoleLoggerProvider::LevelErrorName;
        default:
            return ConsoleLoggerProvider::Yellow + ConsoleLoggerProvider::LevelUnknownName;
    }
}