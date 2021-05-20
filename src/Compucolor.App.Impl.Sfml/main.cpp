#include <memory>

#include <SFML/Graphics.hpp>

#include <Compucolor.App/ConsoleLoggerProvider.h>
#include <Compucolor.App/Emulator.h>

#include "Display.h"
#include "ErrorHandler.h"
#include "KeyboardTranslator.h"

int main(int argc, char** argv) {
    signal(SIGSEGV, Compucolor::App::Impl::Sfml::ErrorHandler);   // install our handler

    // Create the main window
    std::shared_ptr<sf::RenderWindow> window =
        std::shared_ptr<sf::RenderWindow>(
            new sf::RenderWindow(sf::VideoMode(384, 256), "Compucolor II Emulator")
        );

    std::shared_ptr<Compucolor::Logger::ILoggerProvider> loggerProvider = 
        std::shared_ptr<Compucolor::Logger::ILoggerProvider>(
            new Compucolor::App::ConsoleLoggerProvider(
                Compucolor::Logger::LogLevel::Trace
            )
        );

    std::unique_ptr<Compucolor::App::Impl::Sfml::Display> display = 
        std::unique_ptr<Compucolor::App::Impl::Sfml::Display>(
            new Compucolor::App::Impl::Sfml::Display(window)
        );

    std::unique_ptr<Compucolor::App::Emulator> emulator = 
        std::unique_ptr<Compucolor::App::Emulator>(
            new Compucolor::App::Emulator(loggerProvider)
        );

    std::unique_ptr<Compucolor::App::Impl::Sfml::KeyboardTranslator> keyboardTranslator = 
        std::unique_ptr<Compucolor::App::Impl::Sfml::KeyboardTranslator>(
            new Compucolor::App::Impl::Sfml::KeyboardTranslator()
        );

    emulator->SetDisplay(display.get());

    emulator->Start();

    while (window->isOpen())
    {  
        // Process events
        sf::Event event;
        while (window->pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
            {
                emulator->Stop();

                window->close();
            }

            if (event.type == sf::Event::KeyPressed)
            {
                emulator->OnKeyDown(keyboardTranslator->ConvertKey(event.key.code));
            }

            if (event.type == sf::Event::KeyReleased)
            {
                emulator->OnKeyUp(keyboardTranslator->ConvertKey(event.key.code));
            }
        }
    }

    return EXIT_SUCCESS;
}