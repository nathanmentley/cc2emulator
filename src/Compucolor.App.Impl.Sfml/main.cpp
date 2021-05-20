#include <memory>

#include <SFML/Graphics.hpp>

#include "Display.h"
#include "Emulator.h"
#include "ErrorHandler.h"
#include "KeyboardTranslator.h"

int main(int argc, char** argv) {
    signal(SIGSEGV, Compucolor::App::Impl::Smfl::ErrorHandler);   // install our handler

    // Create the main window
    std::shared_ptr<sf::RenderWindow> window =
        std::shared_ptr<sf::RenderWindow>(
            new sf::RenderWindow(sf::VideoMode(384, 256), "Compucolor II Emulator")
        );

    std::unique_ptr<Compucolor::App::Impl::Smfl::Display> display = 
        std::unique_ptr<Compucolor::App::Impl::Smfl::Display>(
            new Compucolor::App::Impl::Smfl::Display(window)
        );

    std::unique_ptr<Compucolor::App::Impl::Smfl::KeyboardTranslator> keyboardTranslator = 
        std::unique_ptr<Compucolor::App::Impl::Smfl::KeyboardTranslator>(
            new Compucolor::App::Impl::Smfl::KeyboardTranslator()
        );

    std::unique_ptr<Compucolor::App::Impl::Smfl::Emulator> emulator = 
        std::unique_ptr<Compucolor::App::Impl::Smfl::Emulator>(
            new Compucolor::App::Impl::Smfl::Emulator()
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