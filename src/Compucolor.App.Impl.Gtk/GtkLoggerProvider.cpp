#include <Compucolor.App.Impl.Gtk/GtkLoggerProvider.h>


#include <chrono> 

GtkLoggerProvider::GtkLoggerProvider()
{
}

void GtkLoggerProvider::Log(LogLevel logLevel, std::string message, va_list args)
{
    time_t timenow = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now()
    ); 

    printf("[%s - %ld] ", GetLogLevel(logLevel).c_str(), timenow);

    vprintf(message.c_str(), args);
    printf("\n");
}

std::string GtkLoggerProvider::GetLogLevel(LogLevel logLevel)
{
    switch(logLevel)
    {
        case LogLevel::Trace:
            return "Trace";
        case LogLevel::Debug:
            return "Debug";
        case LogLevel::Information:
            return "Information";
        case LogLevel::Warning:
            return "Warning";
        case LogLevel::Error:
            return "Error";
    }
}