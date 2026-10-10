#include "Program.h"

using std::optional;
using std::shared_ptr;
using std::make_shared;
using std::make_unique;

using sf::Event;
using sf::RenderWindow;
using sf::Vector2u;
using sf::VideoMode;

using Compucolor::Common::PluginManager;

using Compucolor::Logger::LogLevel;
using Compucolor::Logger::ConsoleLoggerProvider;

using Compucolor::Keyboard::CompucolorIIKey;

using Compucolor::Impl::App::Sfml::Program;

Program::Program():
    _window(make_shared<RenderWindow>(VideoMode(Vector2u(384, 256)), "Compucolor II Emulator")),
    _loggerProvider(make_shared<ConsoleLoggerProvider>(LogLevel::Debug)),
    _configuration(make_shared<Configuration>()),
    _display(make_shared<Display>(_window)),
    _manager(
        make_shared<PluginManager>(
            _loggerProvider,
            _display,
            _configuration->GetSetting("logger_impl").value(),
            _configuration->GetSetting("intel8080_impl").value(),
            _configuration->GetSetting("memory_impl").value(),
            _configuration->GetSetting("scheduler_impl").value(),
            _configuration->GetSetting("keyboard_impl").value(),
            _configuration->GetSetting("floppyA_impl").value(),
            _configuration->GetSetting("floppyB_impl").value(),
            _configuration->GetSetting("smc5027_impl").value(),
            _configuration->GetSetting("crt_impl").value(),
            _configuration->GetSetting("tms5501_impl").value(),
            _configuration->GetSetting("compucolor_impl").value()
        )
    ),
    _emulator(make_unique<CompucolorEmulatorProcessor>(_manager->GetCompucolor())),
    _keyboardTranslator(make_unique<KeyboardTranslator>())
{
}

int Program::Run() const {
    _emulator->Start();

    while (_window->isOpen())
    {
        while (optional<Event> event = _window->pollEvent())
        {
            if (event.has_value()) {
                ProcessEvent(event.value());
            }
        }

        _display->Render();
    }

    _emulator->Stop();
    _display->Close();

    return EXIT_SUCCESS;
}

void Program::ProcessEvent(const Event &event) const
{
    if (event.is<Event::Closed>())
    {
        _window->close();
    }

    if (event.is<Event::KeyPressed>())
    {
        const auto* pressed = event.getIf<Event::KeyPressed>();
        const optional<CompucolorIIKey> key = KeyboardTranslator::ConvertKey(pressed->code);

        // PC Shift+; is ':', which is an unshifted key on the CC2.
        if (pressed->code == sf::Keyboard::Key::Semicolon && pressed->shift)
        {
            _emulator->OnKeyUp(CompucolorIIKey::Shift);
            _emulator->OnKeyDown(CompucolorIIKey::Colon);
        }
        else if (key.has_value())
        {
            _emulator->OnKeyDown(key.value());
        }
    }

    if (event.is<Event::KeyReleased>())
    {
        const auto* released = event.getIf<Event::KeyReleased>();
        const optional<CompucolorIIKey> key = KeyboardTranslator::ConvertKey(released->code);

        if (released->code == sf::Keyboard::Key::Semicolon)
        {
            _emulator->OnKeyUp(CompucolorIIKey::Colon);
            _emulator->OnKeyUp(CompucolorIIKey::SemiColon);

            if (released->shift)
            {
                _emulator->OnKeyDown(CompucolorIIKey::Shift);
            }
        }
        else if (key.has_value())
        {
            _emulator->OnKeyUp(key.value());
        }
    }
}