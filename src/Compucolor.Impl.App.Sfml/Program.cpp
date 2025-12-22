#include "Program.h"

Compucolor::Impl::App::Sfml::Program::Program():
    _window(
        std::shared_ptr<sf::RenderWindow>(
            new sf::RenderWindow(sf::VideoMode(sf::Vector2u(384, 256)), "Compucolor II Emulator")
        )
    ),
    _manager(
        std::shared_ptr<Compucolor::Common::PluginManager>(
            new Compucolor::Common::PluginManager(
                "libCompucolor.Impl.Intel8080",
                "libCompucolor.Impl.Memory"
            )
        )
    ),
    _emulator(
        get_emulator(
            std::shared_ptr<Compucolor::Logger::ILoggerProvider>(
                new Compucolor::Logger::ConsoleLoggerProvider(
                    Compucolor::Logger::LogLevel::Debug
                )
            ),
            std::shared_ptr<Display>(
                new Display(_window)
            ),
            _manager
        )
    ),
    _keyboardTranslator(
        std::unique_ptr<KeyboardTranslator>(
            new KeyboardTranslator()
        )
    )
{
}

int Compucolor::Impl::App::Sfml::Program::Run()
{
    _emulator->Start();

    while (_window->isOpen())
    {  
        while (std::optional<sf::Event> event = _window->pollEvent())
        {
            if (!event.has_value()) {
                ProcessEvent(event.value());
            }
        }
    }

    return EXIT_SUCCESS;
}

void Compucolor::Impl::App::Sfml::Program::ProcessEvent(sf::Event event)
{
    if (event.is<sf::Event::Closed>())
    {
        _emulator->Stop();

        _window->close();
    }

    if (event.is<sf::Event::KeyPressed>())
    {
        std::optional<Compucolor::Keyboard::CompucolorIIKey> key =
            _keyboardTranslator->ConvertKey(event.getIf<sf::Event::KeyPressed>()->code);

        if (key.has_value())
        {
            _emulator->OnKeyDown(key.value());
        }
    }

    if (event.is<sf::Event::KeyReleased>())
    {
        std::optional<Compucolor::Keyboard::CompucolorIIKey> key =
            _keyboardTranslator->ConvertKey(event.getIf<sf::Event::KeyReleased>()->code);

        if (key.has_value())
        {
            _emulator->OnKeyUp(key.value());
        }
    }
}