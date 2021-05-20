#pragma once

#include <chrono>
#include <optional>

#include "ILoggerProvider.h"

namespace Compucolor::Logger
{
    class ConsoleLoggerProvider: public Logger::ILoggerProvider {
        public:
            ConsoleLoggerProvider(Logger::LogLevel logLevel):
                _logLevel(logLevel)
            {
            };

            virtual void Write(Logger::LogLevel logLevel, std::string message, va_list args) override {
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
            };

            virtual void SetLogLevel(Logger::LogLevel level) override
            {
                _logLevel = level;
            };

            virtual Logger::LogLevel GetLogLevel() override
            {
                return _logLevel.value_or(Logger::LogLevel::Error);
            };

        private:
            std::optional<Logger::LogLevel> _logLevel;

            std::string GetLogLevel(Logger::LogLevel logLevel)
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
            inline const static std::string LevelUnknownName = "Unknown Log Level";
    };
}