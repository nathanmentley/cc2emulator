#pragma once

#include <chrono>
#include <optional>

#include <Compucolor.Logger/ILoggerProvider.h>

class GtkLoggerProvider: public ILoggerProvider {
    public:
        GtkLoggerProvider(LogLevel logLevel);

        virtual void Write(LogLevel logLevel, std::string message, va_list args) override;

        virtual void SetLogLevel(LogLevel level) override;

        virtual LogLevel GetLogLevel() override;

    private:
        std::optional<LogLevel> _logLevel;

        std::string GetLogLevel(LogLevel logLevel);

        inline const static std::string Red = "\x1B[31m";
        inline const static std::string Green = "\x1B[32m";
        inline const static std::string Yellow = "\x1B[33m";
        inline const static std::string Blue = "\x1B[34m";
        inline const static std::string Magenta = "\x1B[35m";
        inline const static std::string Cyan = "\x1B[36m";
        inline const static std::string White = "\x1B[37m";
        inline const static std::string Reset = "\x1B[0m";

        inline const static std::string LevelTraceName = "Trace";
        inline const static std::string LevelDebugName = "Debug";
        inline const static std::string LevelInformationName = "Information";
        inline const static std::string LevelWarningName = "Warning";
        inline const static std::string LevelErrorName = "Error";
};