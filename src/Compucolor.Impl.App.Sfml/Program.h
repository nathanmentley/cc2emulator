
#pragma once

#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
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
#include "Display.h"
#include "Gui.h"
#include "KeyboardTranslator.h"
#include "Settings.h"

namespace Compucolor::Impl::App::Sfml
{
    class Program {
        public:
            Program();

            [[nodiscard]] int Run() const;

        private:
            using EmulatedKey = Compucolor::Keyboard::CompucolorIIKey;

            std::string _settingsPath;
            std::shared_ptr<Settings> _settings;
            std::shared_ptr<sf::RenderWindow> _window;
            std::shared_ptr<Compucolor::Logger::ConsoleLoggerProvider> _loggerProvider;
            std::shared_ptr<Compucolor::Common::IConfiguration> _configuration;
            std::shared_ptr<Display> _display;
            std::shared_ptr<AudioOutput> _audio;
            std::shared_ptr<Compucolor::Common::IPluginManager> _manager;
            std::unique_ptr<Compucolor::CompucolorEmulatorProcessor> _emulator;
            std::unique_ptr<Gui> _gui;
            std::unique_ptr<KeyboardTranslator> _keyboardTranslator;

            // Emulated keys currently held by each physical key.
            mutable std::map<sf::Keyboard::Key, std::vector<EmulatedKey>> _pressedKeys;

            // Number of physical sources currently holding each emulated key.
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

            // Alt+Enter fullscreen, Alt+1..4 scale, Alt+S smoothing,
            // Alt+A sound, Alt+M menu, Alt+P screenshot.
            bool HandleHotkey(const sf::Event::KeyPressed& pressed) const;

            void RecreateWindow() const;

            // Presses and releases physical keys while reference-counting
            // their corresponding emulated keys.
            void PressPhysicalKeys(
                sf::Keyboard::Key physical,
                std::vector<EmulatedKey> keys
            ) const;

            void ReleasePhysicalKey(sf::Keyboard::Key physical) const;

            void ReleaseAllKeys() const;

            void ReleaseStuckOneShotKeys() const;

            void PressChord(
                sf::Keyboard::Key physical,
                const KeyChord& chord
            ) const;
    };
}
