#include <memory>
#include <optional>

#include <SFML/Graphics.hpp>

#include <Compucolor/Logger/ConsoleLoggerProvider.h>
#include <Compucolor/EmulatorBackgroundTask.h>

#include <Compucolor.App/get_emulator.h>

#include "Display.h"
#include "ErrorHandler.h"
#include "KeyboardTranslator.h"

namespace Compucolor::App::Impl::Sfml
{
    class Program {
        public:
            Program();

            int Run();

        private:
            std::shared_ptr<sf::RenderWindow> _window;
            std::unique_ptr<EmulatorBackgroundTask> _emulator;
            std::unique_ptr<KeyboardTranslator> _keyboardTranslator;

            void ProcessEvent(sf::Event event);
    };
}