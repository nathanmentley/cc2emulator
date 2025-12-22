#include "Program.h"

#include <memory>

Compucolor::Impl::App::Sfml::Program::Program():
    _window(
        std::make_shared<sf::RenderWindow>(
            sf::VideoMode(sf::Vector2u(384, 256)), "Compucolor II Emulator"
        )
    ),
    _loggerProvider(std::make_shared<Compucolor::Logger::ConsoleLoggerProvider>(Compucolor::Logger::LogLevel::Debug)),
    _configuration(std::make_shared<Configuration>()),
    _display(std::make_shared<Display>(_window)),
    _manager(
        std::make_shared<Common::PluginManager>(
            _loggerProvider,
            _display,
            _configuration->GetSetting("logger_impl").value(),
            _configuration->GetSetting("intel8080_impl").value(),
            _configuration->GetSetting("memory_impl").value(),
            _configuration->GetSetting("scheduler_impl").value(),
            _configuration->GetSetting("keyboard_impl").value(),
            _configuration->GetSetting("floppy_impl").value(),
            _configuration->GetSetting("smc5027_impl").value(),
            _configuration->GetSetting("crt_impl").value(),
            _configuration->GetSetting("tms5501_impl").value(),
            _configuration->GetSetting("compucolor_impl").value()
        )
    ),
    _emulator(get_emulator(_manager)),
    _keyboardTranslator(std::make_unique<KeyboardTranslator>())
{
}

int Compucolor::Impl::App::Sfml::Program::Run() const {
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

void Compucolor::Impl::App::Sfml::Program::ProcessEvent(const sf::Event &event) const
{
    if (event.is<sf::Event::Closed>())
    {
        _emulator->Stop();

        _window->close();
    }

    if (event.is<sf::Event::KeyPressed>())
    {
        const std::optional<Compucolor::Keyboard::CompucolorIIKey> key =
            _keyboardTranslator->ConvertKey(event.getIf<sf::Event::KeyPressed>()->code);

        if (key.has_value())
        {
            _emulator->OnKeyDown(key.value());
        }
    }

    if (event.is<sf::Event::KeyReleased>())
    {
        const std::optional<Compucolor::Keyboard::CompucolorIIKey> key =
            _keyboardTranslator->ConvertKey(event.getIf<sf::Event::KeyReleased>()->code);

        if (key.has_value())
        {
            _emulator->OnKeyUp(key.value());
        }
    }
}