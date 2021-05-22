#include <memory>

#include <SFML/Graphics.hpp>

#include <Compucolor/Logger/ConsoleLoggerProvider.h>

#include "Display.h"
#include "get_emulator.h"
#include "KeyboardTranslator.h"

namespace Compucolor::Impl::App::Sfml
{
    class Program {
        public:
            Program();

            int Run();

        private:
            std::shared_ptr<sf::RenderWindow> _window;
            std::unique_ptr<CompucolorEmulatorProcessor> _emulator;
            std::unique_ptr<KeyboardTranslator> _keyboardTranslator;

            void ProcessEvent(sf::Event event);
    };
}