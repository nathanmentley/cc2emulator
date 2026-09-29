#pragma once

#include <memory>

#include <SFML/Graphics.hpp>

#include <Compucolor/CompucolorEmulatorProcessor.h>
#include <Compucolor/Common/PluginManager.h>
#include <Compucolor/Common/IConfiguration.h>
#include <Compucolor/Logger/ConsoleLoggerProvider.h>

#include "Configuration.h"
#include "Display.h"
#include "KeyboardTranslator.h"

namespace Compucolor::Impl::App::Sfml
{
    class Program {
        public:
            Program();

            [[nodiscard]] int Run() const;

        private:
            std::shared_ptr<sf::RenderWindow> _window;
            std::shared_ptr<Compucolor::Logger::ConsoleLoggerProvider> _loggerProvider;
            std::shared_ptr<Compucolor::Common::IConfiguration> _configuration;
            std::shared_ptr<Display> _display;
            std::shared_ptr<Compucolor::Common::IPluginManager> _manager;
            std::unique_ptr<Compucolor::CompucolorEmulatorProcessor> _emulator;
            std::unique_ptr<KeyboardTranslator> _keyboardTranslator;

            void ProcessEvent(const sf::Event &event) const;
    };
}