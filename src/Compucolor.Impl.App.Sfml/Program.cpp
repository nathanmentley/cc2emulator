#include "Program.h"

#include <algorithm>
#include <stdexcept>

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
    _settingsPath(Settings::DefaultPath()),
    _settings(LoadSettings(_settingsPath)),
    _window(CreateWindow(*_settings)),
    _loggerProvider(make_shared<ConsoleLoggerProvider>(LogLevel::Debug)),
    _configuration(make_shared<Configuration>()),
    _display(make_shared<Display>(_window)),
    _audio(make_shared<AudioOutput>()),
    _manager(
        make_shared<PluginManager>(
            _loggerProvider,
            _display,
            _audio,
            RequireSetting("logger_impl"),
            RequireSetting("intel8080_impl"),
            RequireSetting("memory_impl"),
            RequireSetting("scheduler_impl"),
            RequireSetting("keyboard_impl"),
            RequireSetting("floppyA_impl"),
            RequireSetting("floppyB_impl"),
            RequireSetting("smc5027_impl"),
            RequireSetting("crt_impl"),
            RequireSetting("tms5501_impl"),
            RequireSetting("compucolor_impl")
        )
    ),
    _emulator(make_unique<CompucolorEmulatorProcessor>(_manager->GetCompucolor())),
    _gui(make_unique<Gui>(_window, _settings, *_emulator, _display)),
    _keyboardTranslator(make_unique<KeyboardTranslator>())
{
    _display->SetSmooth(_settings->GetBool("smooth_scaling"));

    _audio->SetVolume(static_cast<float>(std::clamp(_settings->GetInt("sound_volume"), 0, 100)) / 100.0f);
    _audio->SetEnabled(_settings->GetBool("sound_enabled"));

    _gui->SetQuitHandler([window = _window] { window->close(); });
    (void)_gui->Init();
    _gui->ApplyStartupSettings();
}

std::string Program::RequireSetting(const std::string& key) const
{
    if (const optional<std::string> value = _configuration->GetSetting(key))
    {
        return value.value();
    }

    throw std::runtime_error(
        "Missing setting '" + key + "' in app.config. Run the emulator from the directory that contains app.config."
    );
}

std::shared_ptr<Compucolor::Impl::App::Sfml::Settings> Program::LoadSettings(const std::string& path)
{
    auto settings = make_shared<Settings>();
    settings->Load(path);

    return settings;
}

std::shared_ptr<RenderWindow> Program::CreateWindow(const Settings& settings)
{
    const auto scale = static_cast<unsigned int>(std::clamp(settings.GetInt("window_scale"), 1, 8));
    const char* title = "Compucolor II Emulator";

    if (settings.GetBool("fullscreen"))
    {
        return make_shared<RenderWindow>(
            VideoMode::getDesktopMode(),
            title,
            sf::Style::Default,
            sf::State::Fullscreen
        );
    }

    return make_shared<RenderWindow>(VideoMode(Vector2u(384 * scale, 256 * scale)), title);
}

void Program::RecreateWindow() const
{
    const auto scale = static_cast<unsigned int>(std::clamp(_settings->GetInt("window_scale"), 1, 8));
    const char* title = "Compucolor II Emulator";

    // ImGui holds GL resources tied to the old window context.
    _gui->Shutdown();

    if (_settings->GetBool("fullscreen"))
    {
        _window->create(VideoMode::getDesktopMode(), title, sf::Style::Default, sf::State::Fullscreen);
    }
    else
    {
        _window->create(VideoMode(Vector2u(384 * scale, 256 * scale)), title);
    }

    (void)_gui->Init();
}

bool Program::HandleHotkey(const Event::KeyPressed& pressed) const
{
    if (!pressed.alt)
    {
        return false;
    }

    using Key = sf::Keyboard::Key;

    if (pressed.code == Key::Enter)
    {
        _settings->SetBool("fullscreen", !_settings->GetBool("fullscreen"));
        RecreateWindow();
        return true;
    }

    if (pressed.code >= Key::Num1 && pressed.code <= Key::Num4)
    {
        _settings->SetInt("window_scale", static_cast<int>(pressed.code) - static_cast<int>(Key::Num1) + 1);
        _settings->SetBool("fullscreen", false);
        RecreateWindow();
        return true;
    }

    if (pressed.code == Key::S)
    {
        const bool smooth = !_settings->GetBool("smooth_scaling");
        _settings->SetBool("smooth_scaling", smooth);
        _display->SetSmooth(smooth);
        return true;
    }

    if (pressed.code == Key::M)
    {
        _gui->ToggleMenu();
        return true;
    }

    if (pressed.code == Key::P)
    {
        _gui->SaveScreenshot();
        return true;
    }

    return false;
}

int Program::Run() const {
    _emulator->Start();

    sf::Clock frameClock;

    while (_window->isOpen())
    {
        while (optional<Event> event = _window->pollEvent())
        {
            if (event.has_value()) {
                ProcessEvent(event.value());
            }
        }

        const sf::Time elapsed = frameClock.restart();

        _gui->BeginFrame(elapsed);
        _display->Render([this] { _gui->Render(); });

        if (_gui->ConsumeWindowChange())
        {
            RecreateWindow();
        }
    }

    _emulator->Stop();
    _display->Close();
    _gui->Shutdown();

    _settings->Save(_settingsPath);

    return EXIT_SUCCESS;
}

void Program::PressChord(const sf::Keyboard::Key physical, const KeyChord& chord) const
{
    _activeChords[physical] = chord.key;

    if (chord.shift)
    {
        _emulator->OnKeyDown(CompucolorIIKey::Shift);
    }
    else
    {
        _emulator->OnKeyUp(CompucolorIIKey::Shift);
    }

    _emulator->OnKeyDown(chord.key);
}

void Program::ProcessEvent(const Event &event) const
{
    _gui->ProcessEvent(event);

    if (event.is<Event::Closed>())
    {
        _gui->RequestQuit();
    }

    if (event.is<Event::KeyPressed>())
    {
        const auto* pressed = event.getIf<Event::KeyPressed>();

        if (HandleHotkey(*pressed))
        {
            return;
        }

        // Releases always pass through so a key held while the GUI took focus is not left pressed.
        if (_gui->WantsKeyboard())
        {
            return;
        }

        // Digits and punctuation go through their character so PC symbols land on the matching CC2 keys.
        if (const optional<char> character = KeyboardTranslator::ToAscii(pressed->code, pressed->shift))
        {
            if (const optional<KeyChord> chord = AsciiKeyMap::Lookup(character.value()))
            {
                PressChord(pressed->code, chord.value());
            }

            return;
        }

        if (const optional<CompucolorIIKey> key = KeyboardTranslator::ConvertKey(pressed->code))
        {
            _emulator->OnKeyDown(key.value());
        }
    }

    if (event.is<Event::KeyReleased>())
    {
        const auto* released = event.getIf<Event::KeyReleased>();

        if (const auto chord = _activeChords.find(released->code); chord != _activeChords.end())
        {
            _emulator->OnKeyUp(chord->second);
            _activeChords.erase(chord);

            // Put Shift back to what the physical key is doing.
            if (released->shift)
            {
                _emulator->OnKeyDown(CompucolorIIKey::Shift);
            }
            else
            {
                _emulator->OnKeyUp(CompucolorIIKey::Shift);
            }

            return;
        }

        if (const optional<CompucolorIIKey> key = KeyboardTranslator::ConvertKey(released->code))
        {
            _emulator->OnKeyUp(key.value());
        }
    }
}
