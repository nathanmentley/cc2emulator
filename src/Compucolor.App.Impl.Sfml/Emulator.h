#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include <Compucolor.App/get_emulator.h>
#include <Compucolor/Common/IDisplay.h>

#include <Compucolor.App.Impl.Sfml/LoggerProvider.h>

namespace Compucolor::App::Impl::Smfl
{
    class Emulator {
        public:
            Emulator();

            void Start();
            void Stop();

            void SetDisplay(Common::IDisplay* display);
            void OnKeyUp(Keyboard::CompucolorIIKey key);
            void OnKeyDown(Keyboard::CompucolorIIKey key);

        private:
            std::shared_ptr<Logger::ILoggerProvider> _loggerProvider;
            std::shared_ptr<ICompucolorEmulator> _emulator;

            std::atomic<bool> _isRunning;
            std::thread _thread;

            static uint32_t GetColor(Common::Color color);
    };
}