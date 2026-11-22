
#include "Program.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

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
using Compucolor::Impl::App::Sfml::ThemeFromInt;

namespace
{
    constexpr auto OneShotKeyTimeout = std::chrono::milliseconds(200);

    bool IsOneShotPhysicalKey(const sf::Keyboard::Key key)
    {
        // Left Super/Windows is mapped to the CC2 AUTO command.
        return key == sf::Keyboard::Key::LSystem;
    }
}

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
    _gui(make_unique<Gui>(_window, _settings, *_emulator, _display, _audio)),
    _keyboardTranslator(make_unique<KeyboardTranslator>())
{
    _display->SetSmooth(_settings->GetBool("smooth_scaling"));
    _display->SetTheme(ThemeFromInt(_settings->GetInt("theme")));

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
        "Missing setting '" + key +
        "' in app.config. Run the emulator from the directory that contains app.config."
    );
}

std::shared_ptr<Compucolor::Impl::App::Sfml::Settings>
Program::LoadSettings(const std::string& path)
{
    auto settings = make_shared<Settings>();
    settings->Load(path);

    return settings;
}

std::shared_ptr<RenderWindow>
Program::CreateWindow(const Settings& settings)
{
    const auto scale = static_cast<unsigned int>(
        std::clamp(settings.GetInt("window_scale"), 1, 8)
    );

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

    return make_shared<RenderWindow>(
        VideoMode(Vector2u(384 * scale, 256 * scale)),
        title
    );
}

void Program::RecreateWindow() const
{
    const auto scale = static_cast<unsigned int>(
        std::clamp(_settings->GetInt("window_scale"), 1, 8)
    );

    const char* title = "Compucolor II Emulator";

    // A recreated window may lose keyboard focus.
    // Release all physical keys before replacing the window.
    ReleaseAllKeys();

    // ImGui holds GL resources tied to the old window context.
    _gui->Shutdown();

    if (_settings->GetBool("fullscreen"))
    {
        _window->create(
            VideoMode::getDesktopMode(),
            title,
            sf::Style::Default,
            sf::State::Fullscreen
        );
    }
    else
    {
        _window->create(
            VideoMode(Vector2u(384 * scale, 256 * scale)),
            title
        );
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
        _settings->SetBool(
            "fullscreen",
            !_settings->GetBool("fullscreen")
        );

        RecreateWindow();
        return true;
    }

    if (pressed.code >= Key::Num1 && pressed.code <= Key::Num4)
    {
        _settings->SetInt(
            "window_scale",
            static_cast<int>(pressed.code) -
            static_cast<int>(Key::Num1) + 1
        );

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

    if (pressed.code == Key::A)
    {
        _gui->ToggleSound();
        return true;
    }

    return false;
}

void Program::PressPhysicalKeys(
    const sf::Keyboard::Key physical,
    std::vector<EmulatedKey> keys
) const
{
    // Ignore repeated KeyPressed events for an already-held physical key.
    if (_pressedKeys.contains(physical))
    {
        return;
    }

    for (const EmulatedKey key : keys)
    {
        auto& count = _keyPressCounts[key];

        // Only send KeyDown when the first physical source presses this key.
        if (count == 0)
        {
            _emulator->OnKeyDown(key);
        }

        ++count;
    }

    _pressedKeys.emplace(physical, std::move(keys));

    if (IsOneShotPhysicalKey(physical))
    {
        _oneShotKeyPressTimes[physical] = std::chrono::steady_clock::now();
    }
}

void Program::ReleasePhysicalKey(const sf::Keyboard::Key physical) const
{
    const auto physicalIt = _pressedKeys.find(physical);

    if (physicalIt == _pressedKeys.end())
    {
        return;
    }

    for (const EmulatedKey key : physicalIt->second)
    {
        const auto countIt = _keyPressCounts.find(key);

        if (countIt == _keyPressCounts.end())
        {
            continue;
        }

        if (countIt->second > 1)
        {
            --countIt->second;
        }
        else
        {
            // Only send KeyUp when the last physical source releases it.
            _emulator->OnKeyUp(key);
            _keyPressCounts.erase(countIt);
        }
    }

    _pressedKeys.erase(physicalIt);
    _oneShotKeyPressTimes.erase(physical);
}

void Program::ReleaseAllKeys() const
{
    // Every key in this map is currently logically pressed in the emulator.
    for (const auto& [key, count] : _keyPressCounts)
    {
        (void)count;
        _emulator->OnKeyUp(key);
    }

    _pressedKeys.clear();
    _keyPressCounts.clear();
    _oneShotKeyPressTimes.clear();
}

void Program::ReleaseStuckOneShotKeys() const
{
    if (_oneShotKeyPressTimes.empty())
    {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    std::vector<sf::Keyboard::Key> expired;

    for (const auto& [physical, pressedAt] : _oneShotKeyPressTimes)
    {
        if (now - pressedAt >= OneShotKeyTimeout)
        {
            expired.push_back(physical);
        }
    }

    for (const sf::Keyboard::Key physical : expired)
    {
        ReleasePhysicalKey(physical);
    }
}

void Program::PressChord(
    const sf::Keyboard::Key physical,
    const KeyChord& chord
) const
{
    std::vector<EmulatedKey> keys;

    if (chord.shift)
    {
        keys.push_back(CompucolorIIKey::Shift);
    }

    keys.push_back(chord.key);

    PressPhysicalKeys(physical, std::move(keys));
}

void Program::ProcessEvent(const Event& event) const
{
    _gui->ProcessEvent(event);

    if (event.is<Event::Closed>())
    {
        ReleaseAllKeys();
        _gui->RequestQuit();
        return;
    }

    // SFML does not guarantee that we will receive KeyReleased events
    // for keys held when the window loses focus.
    if (event.is<Event::FocusLost>())
    {
        ReleaseAllKeys();
        return;
    }

    if (event.is<Event::KeyPressed>())
    {
        const auto* pressed = event.getIf<Event::KeyPressed>();

        if (HandleHotkey(*pressed))
        {
            return;
        }

        // Don't forward new keyboard input while the GUI wants it.
        // Existing keys are still released below, regardless of GUI focus.
        if (_gui->WantsKeyboard())
        {
            return;
        }

        // Shift is part of the chord resolved from the typed character below.
        // Forwarding it here as well would double-apply it: PC Shift+Semicolon
        // resolves to ':', but CC2 Shift+Colon is '*'.
        if (pressed->code == sf::Keyboard::Key::LShift ||
            pressed->code == sf::Keyboard::Key::RShift)
        {
            return;
        }

        // Digits and punctuation go through their character so PC symbols
        // land on the matching CC2 keys.
        if (const optional<char> character =
            KeyboardTranslator::ToAscii(pressed->code, pressed->shift))
        {
            if (const optional<KeyChord> chord =
                AsciiKeyMap::Lookup(character.value()))
            {
                PressChord(pressed->code, chord.value());
            }

            return;
        }

        if (const optional<CompucolorIIKey> key =
            KeyboardTranslator::ConvertKey(pressed->code))
        {
            PressPhysicalKeys(
                pressed->code,
                { key.value() }
            );
        }

        return;
    }

    if (event.is<Event::KeyReleased>())
    {
        const auto* released = event.getIf<Event::KeyReleased>();

        // Only release keys that we actually forwarded on KeyPressed.
        // This also works if the GUI has since taken keyboard focus.
        ReleasePhysicalKey(released->code);
    }
}

int Program::Run() const
{
    _emulator->Start();

    sf::Clock frameClock;

    while (_window->isOpen())
    {
        while (optional<Event> event = _window->pollEvent())
        {
            if (event.has_value())
            {
                ProcessEvent(event.value());
            }
        }

        ReleaseStuckOneShotKeys();

        const sf::Time elapsed = frameClock.restart();

        _gui->BeginFrame(elapsed);
        _display->Render([this] { _gui->Render(); });

        if (_gui->ConsumeWindowChange())
        {
            RecreateWindow();
        }
    }

    ReleaseAllKeys();

    _emulator->Stop();
    _display->Close();
    _gui->Shutdown();

    _settings->Save(_settingsPath);

    return EXIT_SUCCESS;
}
