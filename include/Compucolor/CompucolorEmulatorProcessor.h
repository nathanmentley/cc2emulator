#pragma once

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <fstream>
#include <future>
#include <iterator>
#include <stdexcept>
#include <string>

#include <boost/json.hpp>

#include <Compucolor/ICompucolorEmulator.h>

namespace Compucolor
{
    // Host-side façade for ICompucolorEmulator: owns the startup thread and
    // persists complete emulator state at caller-supplied paths.
    class CompucolorEmulatorProcessor {
        public:
            CompucolorEmulatorProcessor(
                std::shared_ptr<ICompucolorEmulator> emulator,
                std::shared_ptr<Debug::IDebugSink> debugSink = nullptr
            ):
                _emulator(std::move(emulator)),
                _debugSink(std::move(debugSink)),
                _thread({})
            {
                if (_debugSink)
                {
                    _emulator->SetDebugSink(_debugSink);
                }
            };

            // Stops the emulator if still running.
            ~CompucolorEmulatorProcessor()
            {
                Stop();
            }

            // Starts ICompucolorEmulator::Start() on a dedicated thread and
            // returns only after initialization is complete. The thread stays
            // alive as the scheduler's owner until Stop() signals it.
            void Start()
            {
                if (_thread.has_value())
                {
                    return;
                }

                std::promise<void> started;
                _started = started.get_future();

                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    _stopRequested = false;
                }

                _thread = std::thread(
                    [this, started = std::move(started)]() mutable {
                        try
                        {
                            _emulator->Start();
                            started.set_value();
                        }
                        catch (...)
                        {
                            started.set_exception(std::current_exception());
                            return;
                        }

                        std::unique_lock<std::mutex> lock(_mutex);
                        _condition.wait(lock, [this] {
                            return _stopRequested;
                        });
                    }
                );

                // Callers load saved state immediately after Start(); emulator
                // startup reloads the ROM, so it must finish first.
                _started.get();
            };

            // Signals the keep-alive thread, joins it, then stops the machine.
            void Stop()
            {
                if (!_thread.has_value())
                {
                    return;
                }

                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    _stopRequested = true;
                }

                _condition.notify_all();
                _thread->join();
                _thread.reset();

                _emulator->Stop();
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

            [[nodiscard]] uint64_t GetEmulatedNanoseconds() const
            {
                return _emulator->GetEmulatedNanoseconds();
            }

            void SetDebugSink(std::shared_ptr<Debug::IDebugSink> sink)
            {
                _debugSink = std::move(sink);
                _emulator->SetDebugSink(_debugSink);
            }

            void SetLowercaseCharset(bool lowercase) { _emulator->SetLowercaseCharset(lowercase); }

            void SetRomVersion(RomVersion version) { _emulator->SetRomVersion(version); }

            // Captures the composite state as a JSON document without writing a file.
            [[nodiscard]] boost::json::value GetState()
            {
                auto state = _emulator->SaveState();

                if (!state)
                {
                    throw std::runtime_error("Emulator returned an empty save state");
                }

                return *state;
            }

            // Serializes the composite state to an explicit path. Throws if the
            // state is empty or the file cannot be written.
            void SaveState(const std::string& path)
            {
                const boost::json::value state = GetState();

                std::ofstream file(path, std::ios::out | std::ios::trunc | std::ios::binary);

                if (!file.is_open())
                {
                    throw std::runtime_error("Unable to open " + path + " for writing");
                }

                file << boost::json::serialize(state);

                if (!file.good())
                {
                    throw std::runtime_error("Failed to write " + path);
                }
            }

            // Loads an already-parsed state document, used by replay bundles.
            void LoadState(const boost::json::value& state)
            {
                _emulator->LoadState(
                    std::make_shared<boost::json::value>(state)
                );
            }

            // Parses an explicit state file. Throws if the file cannot be read;
            // invalid contents are rejected by the emulator's state loader.
            void LoadState(const std::string& path)
            {
                std::ifstream file(path, std::ios::in | std::ios::binary);

                if (!file.is_open())
                {
                    throw std::runtime_error("Unable to open " + path + " for reading");
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
            std::shared_ptr<Debug::IDebugSink> _debugSink;
            std::optional<std::thread> _thread;
            std::future<void> _started;

            std::mutex _mutex;
            std::condition_variable _condition;
            bool _stopRequested = false;
    };
}
