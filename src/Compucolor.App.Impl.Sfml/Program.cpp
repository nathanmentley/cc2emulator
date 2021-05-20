#include "Program.h"

Compucolor::App::Impl::Sfml::Program::Program():
    _window(
        std::shared_ptr<sf::RenderWindow>(
            new sf::RenderWindow(sf::VideoMode(384, 256), "Compucolor II Emulator")
        )
    ),
    _emulator(
        get_emulator(
            std::shared_ptr<Logger::ILoggerProvider>(
                new Logger::ConsoleLoggerProvider(
                    Logger::LogLevel::Trace
                )
            ),
            std::shared_ptr<Display>(
                new Display(_window)
            )
        )
    ),
    _keyboardTranslator(
        std::unique_ptr<KeyboardTranslator>(
            new KeyboardTranslator()
        )
    )
{
}

int Compucolor::App::Impl::Sfml::Program::Run()
{
    _emulator->Start();

    while (_window->isOpen())
    {  
        sf::Event event;
        while (_window->pollEvent(event))
        {
            ProcessEvent(event);
        }
    }

    return EXIT_SUCCESS;
}

void Compucolor::App::Impl::Sfml::Program::ProcessEvent(sf::Event event)
{
    if (event.type == sf::Event::Closed)
    {
        _emulator->Stop();

        _window->close();
    }

    if (event.type == sf::Event::KeyPressed)
    {
        std::optional<Keyboard::CompucolorIIKey> key =
            _keyboardTranslator->ConvertKey(event.key.code);

        if (key.has_value())
        {
            _emulator->OnKeyDown(key.value());
        }
    }

    if (event.type == sf::Event::KeyReleased)
    {
        std::optional<Keyboard::CompucolorIIKey> key =
            _keyboardTranslator->ConvertKey(event.key.code);

        if (key.has_value())
        {
            _emulator->OnKeyUp(key.value());
        }
    }
}