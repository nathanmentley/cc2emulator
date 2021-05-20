#include <Compucolor.App.Impl.Gtk/GtkLoggerProvider.h>

GtkLoggerProvider::GtkLoggerProvider(Compucolor::Logger::LogLevel logLevel):
    _logLevel(logLevel)
{
}

void GtkLoggerProvider::Write(Compucolor::Logger::LogLevel logLevel, std::string message, va_list args)
{
    Compucolor::Logger::LogLevel currentLevel = _logLevel
        .value_or(Compucolor::Logger::LogLevel::Error);

    if (logLevel < currentLevel)
    {
        return;
    }

    time_t timenow = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now()
    );

    printf(
        "%s[%s%s - %s%ld%s]%s ",
        GtkLoggerProvider::Blue.c_str(),
        GetLogLevel(logLevel).c_str(),
        GtkLoggerProvider::Blue.c_str(),
        GtkLoggerProvider::Green.c_str(),
        timenow,
        GtkLoggerProvider::Blue.c_str(),
        GtkLoggerProvider::Yellow.c_str()
    );

    vprintf(message.c_str(), args);
    printf("%s\n", GtkLoggerProvider::Reset.c_str());
}

void GtkLoggerProvider::SetLogLevel(Compucolor::Logger::LogLevel level)
{
    _logLevel = level;
}

Compucolor::Logger::LogLevel GtkLoggerProvider::GetLogLevel()
{
    return _logLevel.value_or(Compucolor::Logger::LogLevel::Error);
}

std::string GtkLoggerProvider::GetLogLevel(Compucolor::Logger::LogLevel logLevel)
{
    switch(logLevel)
    {
        case Compucolor::Logger::LogLevel::Trace:
            return GtkLoggerProvider::White + GtkLoggerProvider::LevelTraceName;
        case Compucolor::Logger::LogLevel::Debug:
            return GtkLoggerProvider::Cyan + GtkLoggerProvider::LevelDebugName;
        case Compucolor::Logger::LogLevel::Information:
            return GtkLoggerProvider::Magenta + GtkLoggerProvider::LevelInformationName;
        case Compucolor::Logger::LogLevel::Warning:
            return GtkLoggerProvider::Yellow + GtkLoggerProvider::LevelWarningName;
        case Compucolor::Logger::LogLevel::Error:
            return GtkLoggerProvider::Red + GtkLoggerProvider::LevelErrorName;
        default:
            return GtkLoggerProvider::Yellow + GtkLoggerProvider::LevelUnknownName;
    }
}