#include <Compucolor.App.Impl.Gtk/GtkLoggerProvider.h>

GtkLoggerProvider::GtkLoggerProvider(LogLevel logLevel):
    _logLevel(logLevel)
{
}

void GtkLoggerProvider::Write(LogLevel logLevel, std::string message, va_list args)
{
    LogLevel currentLevel = _logLevel.value_or(LogLevel::Error);

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

void GtkLoggerProvider::SetLogLevel(LogLevel level)
{
    _logLevel = level;
}

LogLevel GtkLoggerProvider::GetLogLevel()
{
    return _logLevel.value_or(LogLevel::Error);
}

std::string GtkLoggerProvider::GetLogLevel(LogLevel logLevel)
{
    switch(logLevel)
    {
        case LogLevel::Trace:
            return GtkLoggerProvider::White + GtkLoggerProvider::LevelTraceName;
        case LogLevel::Debug:
            return GtkLoggerProvider::Cyan + GtkLoggerProvider::LevelDebugName;
        case LogLevel::Information:
            return GtkLoggerProvider::Magenta + GtkLoggerProvider::LevelInformationName;
        case LogLevel::Warning:
            return GtkLoggerProvider::Yellow + GtkLoggerProvider::LevelWarningName;
        case LogLevel::Error:
            return GtkLoggerProvider::Red + GtkLoggerProvider::LevelErrorName;
        default:
            return GtkLoggerProvider::Yellow + GtkLoggerProvider::LevelUnknownName;
    }
}