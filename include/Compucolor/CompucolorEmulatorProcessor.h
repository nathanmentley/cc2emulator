#pragma once

#include <memory>
#include <optional>
#include <thread>
#include <utility>

#include <Compucolor/ICompucolorEmulator.h>

namespace Compucolor
{
    class CompucolorEmulatorProcessor {
        public:
            CompucolorEmulatorProcessor(
                std::shared_ptr<ICompucolorEmulator> emulator
            ):
                _emulator(std::move(emulator)),
                _thread({})
            {
            };

            ~CompucolorEmulatorProcessor()
            {
                Stop();
            }

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
                    _thread->join();

                    _emulator->Stop();

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

            void Reset(bool hard) { _emulator->Reset(hard); }

            void Pause() { _emulator->Pause(); }

            void Resume() { _emulator->Resume(); }

            [[nodiscard]] bool IsPaused() const { return _emulator->IsPaused(); }

            void SetSpeedRegulated(bool regulated) { _emulator->SetSpeedRegulated(regulated); }

            [[nodiscard]] bool IsSpeedRegulated() const { return _emulator->IsSpeedRegulated(); }

            void SetLowercaseCharset(bool lowercase) { _emulator->SetLowercaseCharset(lowercase); }

            void SetRomVersion(RomVersion version) { _emulator->SetRomVersion(version); }

            [[nodiscard]] std::shared_ptr<Floppy::IFloppyEmulator> GetFloppy(int drive) const
            {
                return _emulator->GetFloppy(drive);
            }

        private:
            std::shared_ptr<ICompucolorEmulator> _emulator;
            std::optional<std::thread> _thread;
    };
}