#include <Compucolor.App.Impl.Sfml/LoggerProvider.h>

Compucolor::App::Impl::Smfl::LoggerProvider::LoggerProvider(Logger::LogLevel logLevel):
    _logLevel(logLevel)
{
}

void Compucolor::App::Impl::Smfl::LoggerProvider::Write(Logger::LogLevel logLevel, std::string message, va_list args)
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
        LoggerProvider::Blue.c_str(),
        GetLogLevel(logLevel).c_str(),
        LoggerProvider::Blue.c_str(),
        LoggerProvider::Green.c_str(),
        timenow,
        LoggerProvider::Blue.c_str(),
        LoggerProvider::Yellow.c_str()
    );

    vprintf(message.c_str(), args);
    printf("%s\n", LoggerProvider::Reset.c_str());
}

void Compucolor::App::Impl::Smfl::LoggerProvider::SetLogLevel(Logger::LogLevel level)
{
    _logLevel = level;
}

Compucolor::Logger::LogLevel Compucolor::App::Impl::Smfl::LoggerProvider::GetLogLevel()
{
    return _logLevel.value_or(Logger::LogLevel::Error);
}

std::string Compucolor::App::Impl::Smfl::LoggerProvider::GetLogLevel(Logger::LogLevel logLevel)
{
    switch(logLevel)
    {
        case Logger::LogLevel::Trace:
            return LoggerProvider::White + LoggerProvider::LevelTraceName;
        case Logger::LogLevel::Debug:
            return LoggerProvider::Cyan + LoggerProvider::LevelDebugName;
        case Logger::LogLevel::Information:
            return LoggerProvider::Magenta + LoggerProvider::LevelInformationName;
        case Logger::LogLevel::Warning:
            return LoggerProvider::Yellow + LoggerProvider::LevelWarningName;
        case Logger::LogLevel::Error:
            return LoggerProvider::Red + LoggerProvider::LevelErrorName;
        default:
            return LoggerProvider::Yellow + LoggerProvider::LevelUnknownName;
    }
}