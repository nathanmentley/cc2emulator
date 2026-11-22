#pragma once

#include <memory>
#include <optional>
#include <thread>
#include <utility>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

#include <boost/json.hpp>

#include <Compucolor/ICompucolorEmulator.h>

namespace Compucolor
{
    // Host-side façade for ICompucolorEmulator: owns the startup thread and
    // persists complete emulator state in ./compucolor-state.json.
    class CompucolorEmulatorProcessor {
        public:
            CompucolorEmulatorProcessor(
                std::shared_ptr<ICompucolorEmulator> emulator
            ):
                _emulator(std::move(emulator)),
                _thread({})
            {
            };

            // Stops the emulator if still running.
            ~CompucolorEmulatorProcessor()
            {
                Stop();
            }

            // Starts ICompucolorEmulator::Start() on a dedicated thread.
            // The machine itself performs its work on scheduler threads, so this
            // thread mainly keeps those tasks alive until Stop() is called.
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

            // Joins the startup thread, then stops the machine.
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

            // Serializes the composite state to compucolor-state.json in the
            // current working directory. Throws if the state is empty or the
            // file cannot be written.
            void SaveState()
            {
                auto state = _emulator->SaveState();
            
                if (!state)
                {
                    throw std::runtime_error("Emulator returned an empty save state");
                }
            
                std::ofstream file("compucolor-state.json");
            
                if (!file.is_open())
                {
                    throw std::runtime_error("Unable to open compucolor-state.json for writing");
                }
            
                file << boost::json::serialize(*state);
            
                if (!file.good())
                {
                    throw std::runtime_error("Failed to write compucolor-state.json");
                }
            }

            // Parses the same file and forwards the document to the emulator.
            // Throws if the file cannot be read; invalid contents are rejected
            // by the emulator's state loader.
            void LoadState()
            {
                std::ifstream file("compucolor-state.json");
            
                if (!file.is_open())
                {
                    throw std::runtime_error("Unable to open compucolor-state.json for reading");
                }
            
                std::string contents{
                    std::istreambuf_iterator<char>(file),
                    std::istreambuf_iterator<char>()
                };
            
                auto state = std::make_shared<boost::json::value>(
                    boost::json::parse(contents)
                );
            
                _emulator->LoadState(state);
            }


            // Drive 0 is A, drive 1 is B.
            [[nodiscard]] std::shared_ptr<Floppy::IFloppyEmulator> GetFloppy(int drive) const
            {
                return _emulator->GetFloppy(drive);
            }

        private:
            std::shared_ptr<ICompucolorEmulator> _emulator;
            std::optional<std::thread> _thread;
    };
}
