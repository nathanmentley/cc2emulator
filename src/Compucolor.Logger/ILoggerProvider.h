#pragma once

#include <stdarg.h>
#include <string>

#include <Compucolor.Logger/LogLevel.h>

class ILoggerProvider {
    public:
        virtual ~ILoggerProvider() {}

        virtual void Log(LogLevel logLevel, std::string message, va_list args) = 0;
};