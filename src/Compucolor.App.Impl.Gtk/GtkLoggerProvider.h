#pragma once

#include <Compucolor.Logger/ILoggerProvider.h>

class GtkLoggerProvider: public ILoggerProvider {
    public:
        GtkLoggerProvider();

        virtual void Log(LogLevel logLevel, std::string message, va_list args) override;

    private:
        std::string GetLogLevel(LogLevel logLevel);
};