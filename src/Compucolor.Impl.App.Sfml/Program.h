#pragma once

#include <map>
#include <memory>

#include <SFML/Graphics.hpp>

#include <Compucolor/CompucolorEmulatorProcessor.h>
#include <Compucolor/Common/PluginManager.h>
#include <Compucolor/Common/IConfiguration.h>
#include <Compucolor/Logger/ConsoleLoggerProvider.h>

#include "Configuration.h"
#include "AsciiKeyMap.h"
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
            std::string _settingsPath;
            std::shared_ptr<Settings> _settings;
            std::shared_ptr<sf::RenderWindow> _window;
            std::shared_ptr<Compucolor::Logger::ConsoleLoggerProvider> _loggerProvider;
            std::shared_ptr<Compucolor::Common::IConfiguration> _configuration;
            std::shared_ptr<Display> _display;
            std::shared_ptr<Compucolor::Common::IPluginManager> _manager;
            std::unique_ptr<Compucolor::CompucolorEmulatorProcessor> _emulator;
            std::unique_ptr<Gui> _gui;
            std::unique_ptr<KeyboardTranslator> _keyboardTranslator;
            mutable std::map<sf::Keyboard::Key, Compucolor::Keyboard::CompucolorIIKey> _activeChords;

            void ProcessEvent(const sf::Event &event) const;

            std::string RequireSetting(const std::string& key) const;

            static std::shared_ptr<Settings> LoadSettings(const std::string& path);
            static std::shared_ptr<sf::RenderWindow> CreateWindow(const Settings& settings);

            // Alt+Enter fullscreen, Alt+1..4 scale, Alt+S smoothing, Alt+M menu, Alt+P screenshot; returns true if handled.
            bool HandleHotkey(const sf::Event::KeyPressed& pressed) const;
            void RecreateWindow() const;

            // Presses a key with the shift state it needs, remembering it for the matching release.
            void PressChord(sf::Keyboard::Key physical, const KeyChord& chord) const;
    };
}