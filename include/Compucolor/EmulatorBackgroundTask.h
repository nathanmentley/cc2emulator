#pragma once

#include <memory>
#include <optional>
#include <thread>

#include <Compucolor/ICompucolorEmulator.h>

namespace Compucolor
{
    class EmulatorBackgroundTask {
        public:
            EmulatorBackgroundTask(
                std::shared_ptr<ICompucolorEmulator> emulator
            ):
                _emulator(emulator),
                _thread({})
            {
            };

            void Start()
            {
                if (!_thread.has_value())
                {
                    _thread = std::thread(
                        [this]() {
                            _emulator->Start();
                        }
                    );
                }
            };

            void Stop()
            {
                if (_thread.has_value())
                {
                    _emulator->Stop();

                    _thread->join();

                    _thread.reset();
                }
            };

            void OnKeyUp(Keyboard::CompucolorIIKey key)
            {
                _emulator->OnKeyUp(key);
            };

            void OnKeyDown(Keyboard::CompucolorIIKey key)
            {
                _emulator->OnKeyDown(key);
            };

        private:
            std::shared_ptr<ICompucolorEmulator> _emulator;
            std::optional<std::thread> _thread;
    };
}