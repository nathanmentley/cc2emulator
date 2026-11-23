
#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include <Compucolor/CompucolorEmulatorProcessor.h>
#include <Compucolor/Common/PluginManager.h>
#include <Compucolor/Common/IConfiguration.h>
#include <Compucolor/Logger/ConsoleLoggerProvider.h>
#include <Compucolor/Keyboard/CompucolorIIKey.h>

#include "Configuration.h"
#include "AsciiKeyMap.h"
#include "AudioOutput.h"
#include <Compucolor/App/DebugHub.h>
#include <Compucolor/App/DebugMonitor.h>
#include "Display.h"
#include <Compucolor/App/JsonlDebugSink.h>
#include "Gui.h"
#include <Compucolor/App/Replay/InputRecorder.h>
#include "KeyboardTranslator.h"
#include "ProgramArguments.h"
#include <Compucolor/App/Replay/ReplayEvent.h>
#include <Compucolor/App/Replay/ReplayInputSink.h>
#include "Settings.h"

namespace Compucolor::Impl::App::Sfml
{
    enum class HotkeyAction;

    // SFML host application. Construction loads settings/config, creates the
    // window and host services, builds the plugin manager and initializes the GUI.
    class Program {
        public:
            explicit Program(ProgramArguments arguments = {});

            // Starts the emulator and runs the event/render loop until the window
            // closes, then releases keys, stops emulation and saves settings.
            int Run();

            // Used by the File menu replay-recording actions.
            void StartReplayRecording(const std::string& path);
            void StopReplayRecording();
            [[nodiscard]] bool IsReplayRecording() const;

        private:
            using EmulatedKey = Compucolor::Keyboard::CompucolorIIKey;
            using VirtualKeySource = std::uint32_t;

            std::string _settingsPath;
            ProgramArguments _arguments;
            std::string _recordingPath;
            bool _wasReplayActive = false;
            std::shared_ptr<Settings> _settings;
            std::shared_ptr<sf::RenderWindow> _window;
            std::shared_ptr<Compucolor::Logger::ConsoleLoggerProvider> _loggerProvider;
            std::shared_ptr<Compucolor::Common::IConfiguration> _configuration;
            std::shared_ptr<Display> _display;
            std::shared_ptr<AudioOutput> _audio;
            std::shared_ptr<Compucolor::Common::IPluginManager> _manager;
            std::shared_ptr<DebugMonitor> _debugMonitor;
            std::shared_ptr<JsonlDebugSink> _jsonlSink;
            std::shared_ptr<DebugHub> _debugHub;
            std::shared_ptr<Compucolor::Impl::App::Replay::InputRecorder> _inputRecorder;
            std::shared_ptr<Compucolor::Impl::App::Replay::ReplayInputSink> _replaySink;
            std::unique_ptr<Compucolor::CompucolorEmulatorProcessor> _emulator;
            std::unique_ptr<Gui> _gui;
            std::unique_ptr<KeyboardTranslator> _keyboardTranslator;

            // Emulated keys currently held by each physical key.
            mutable std::map<sf::Keyboard::Key, std::vector<EmulatedKey>> _pressedKeys;

            // Emulated keys held by the mouse-driven on-screen keyboard. Each
            // button gets its own source so modifier latches can stay down
            // independently of one-shot key taps.
            mutable std::map<VirtualKeySource, std::vector<EmulatedKey>> _pressedVirtualKeys;

            // Number of physical or virtual sources holding each emulated key.
            mutable std::map<EmulatedKey, std::size_t> _keyPressCounts;

            // Some desktop environments swallow the release of the Super key,
            // which is mapped to the CC2 AUTO command. AUTO is a one-shot
            // command, so release it automatically if its keyup never arrives.
            mutable std::map<sf::Keyboard::Key, std::chrono::steady_clock::time_point>
                _oneShotKeyPressTimes;

            void ProcessEvent(const sf::Event& event) const;

            std::string RequireSetting(const std::string& key) const;

            static std::shared_ptr<Settings> LoadSettings(const std::string& path);
            static std::shared_ptr<sf::RenderWindow> CreateWindow(const Settings& settings);

            // Alt is reserved for host shortcuts (see Hotkeys.h). Returns
            // true when the event must not reach the emulated keyboard,
            // including unbound Alt chords.
            bool HandleHotkey(const sf::Event::KeyPressed& pressed) const;

            void DispatchHotkey(HotkeyAction action) const;

            void RecreateWindow() const;

            // Presses and releases physical keys while reference-counting
            // their corresponding emulated keys.
            void PressPhysicalKeys(
                sf::Keyboard::Key physical,
                std::vector<EmulatedKey> keys
            ) const;

            void ReleasePhysicalKey(sf::Keyboard::Key physical) const;

            // GUI-driven key injection. Replay owns the keyboard while active,
            // so virtual presses obey the same gate as host key events.
            void InjectVirtualKey(VirtualKeySource source, EmulatedKey key, bool down) const;

            void PressVirtualKeys(VirtualKeySource source, std::vector<EmulatedKey> keys) const;

            void ReleaseVirtualKey(VirtualKeySource source) const;

            void ReleaseAllKeys() const;

            void ReleaseStuckOneShotKeys() const;

            void PressChord(
                sf::Keyboard::Key physical,
                const KeyChord& chord
            ) const;

            void ApplyReplayEvent(const Compucolor::Impl::App::Replay::ReplayEvent& event) const;
            void ApplyStartupReplayOptions();
    };
}
