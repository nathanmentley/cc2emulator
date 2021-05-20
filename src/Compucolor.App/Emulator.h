#pragma once

#include <memory>
#include <optional>
#include <thread>

#include <Compucolor.App/get_emulator.h>
#include <Compucolor/Common/IDisplay.h>
#include <Compucolor/Logger/ILoggerProvider.h>

namespace Compucolor::App
{
    class Emulator {
        public:
            Emulator(
                std::shared_ptr<Logger::ILoggerProvider> loggerProvider
            );

            void Start();
            void Stop();

            void SetDisplay(Common::IDisplay* display);
            void OnKeyUp(Keyboard::CompucolorIIKey key);
            void OnKeyDown(Keyboard::CompucolorIIKey key);

        private:
            std::shared_ptr<Logger::ILoggerProvider> _loggerProvider;
            std::shared_ptr<ICompucolorEmulator> _emulator;
            std::optional<std::thread> _thread;

            static uint32_t GetColor(Common::Color color);
    };
}