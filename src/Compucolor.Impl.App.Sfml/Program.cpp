
#include "Program.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
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

Program::Program(ProgramArguments arguments):
    _settingsPath(Settings::DefaultPath()),
    _arguments(std::move(arguments)),
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
    _debugMonitor(make_shared<DebugMonitor>(_manager->GetMemory())),
    _jsonlSink(
        _arguments.traceJsonlPath.empty()
            ? nullptr
            : make_shared<JsonlDebugSink>(
                _arguments.traceJsonlPath,
                _arguments.traceFilters
            )
    ),
    _debugHub(make_shared<DebugHub>()),
    _inputRecorder(make_shared<Compucolor::Impl::App::Replay::InputRecorder>()),
    _replaySink(make_shared<Compucolor::Impl::App::Replay::ReplayInputSink>()),
    _emulator(make_unique<CompucolorEmulatorProcessor>(
        _manager->GetCompucolor(),
        _debugHub
    )),
    _gui(make_unique<Gui>(
        _window,
        _settings,
        *_emulator,
        _debugMonitor,
        _debugHub,
        _display,
        _audio
    )),
    _keyboardTranslator(make_unique<KeyboardTranslator>())
{
    _debugHub->AddSink(_debugMonitor);
    _debugHub->AddSink(_replaySink);

    if (_jsonlSink)
    {
        _debugHub->AddSink(_jsonlSink);
    }

    // A CLI trace request starts recording immediately; otherwise recording
    // stays off until the Debug menu enables it.
    _debugHub->SetEnabled(_jsonlSink != nullptr);
    _debugMonitor->SetEnabled(_jsonlSink != nullptr);

    _display->SetSmooth(_settings->GetBool("smooth_scaling"));
    _display->SetTheme(ThemeFromInt(_settings->GetInt("theme")));

    _gui->SetQuitHandler([window = _window] { window->close(); });
    _gui->SetReplayHandlers(
        [this](const std::string& path) { StartReplayRecording(path); },
        [this] { StopReplayRecording(); }
    );

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
            _inputRecorder->Record(
                true,
                key,
                _emulator->GetEmulatedNanoseconds()
            );
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
            _inputRecorder->Record(
                false,
                key,
                _emulator->GetEmulatedNanoseconds()
            );
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
        _inputRecorder->Record(
            false,
            key,
            _emulator->GetEmulatedNanoseconds()
        );
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

        // Replay is the only source of emulated key events while it is active.
        // Host hotkeys and GUI navigation above this point still work.
        if (_replaySink->IsActive())
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
        if (_replaySink->IsActive())
        {
            return;
        }

        const auto* released = event.getIf<Event::KeyReleased>();

        // Only release keys that we actually forwarded on KeyPressed.
        // This also works if the GUI has since taken keyboard focus.
        ReleasePhysicalKey(released->code);
    }
}

void Program::ApplyReplayEvent(const Compucolor::Impl::App::Replay::ReplayEvent& event) const
{
    if (event.down)
    {
        _emulator->OnKeyDown(event.key);
    }
    else
    {
        _emulator->OnKeyUp(event.key);
    }
}

void Program::ApplyStartupReplayOptions()
{
    if (!_arguments.loadStatePath.empty())
    {
        _emulator->LoadState(_arguments.loadStatePath);
        _gui->ShowMessage("Loaded state from " + _arguments.loadStatePath);
    }

    if (!_arguments.replayBundlePath.empty())
    {
        const auto bundle = Compucolor::Impl::App::Replay::ReplayBundle::ParseFile(
            _arguments.replayBundlePath
        );

        // Replay owns the keyboard from here on; don't leave held keys behind.
        ReleaseAllKeys();

        _emulator->LoadState(bundle.State());
        _replaySink->Start(
            bundle,
            [this](const Compucolor::Impl::App::Replay::ReplayEvent& event) {
                ApplyReplayEvent(event);
            }
        );

        _gui->ShowMessage(
            "Replaying " + _arguments.replayBundlePath + " - live input paused"
        );
    }

    if (!_arguments.recordReplayPath.empty())
    {
        StartReplayRecording(_arguments.recordReplayPath);
    }
}

void Program::StartReplayRecording(const std::string& path)
{
    try
    {
        if (path.empty())
        {
            throw std::runtime_error("Replay bundle path is empty");
        }

        if (_inputRecorder->IsRecording())
        {
            throw std::runtime_error("Replay recording is already active");
        }

        if (_replaySink->IsActive())
        {
            throw std::runtime_error("Stop the active replay before recording");
        }

        _inputRecorder->Start(
            _emulator->GetState(),
            _emulator->GetEmulatedNanoseconds()
        );

        _recordingPath = path;
        _gui->ShowMessage("Recording replay to " + path);
    }
    catch (const std::exception& error)
    {
        _gui->ReportError(std::string("Unable to start replay recording: ") + error.what());
    }
}

void Program::StopReplayRecording()
{
    if (!_inputRecorder->IsRecording())
    {
        return;
    }

    // Flush held keys so the bundle does not end mid-chord.
    ReleaseAllKeys();

    try
    {
        const auto bundle = _inputRecorder->BuildBundle(
            _emulator->GetEmulatedNanoseconds(),
            "cc2emu"
        );

        std::ofstream file(_recordingPath, std::ios::out | std::ios::trunc | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("Unable to open " + _recordingPath + " for writing");
        }

        file << boost::json::serialize(bundle.ToJson());

        if (!file.good())
        {
            throw std::runtime_error("Failed to write " + _recordingPath);
        }

        const std::string path = _recordingPath;

        _inputRecorder->Stop();
        _recordingPath.clear();
        _gui->ShowMessage("Saved replay bundle " + path);
    }
    catch (const std::exception& error)
    {
        _gui->ReportError(std::string("Unable to save replay bundle: ") + error.what());
    }
}

bool Program::IsReplayRecording() const
{
    return _inputRecorder->IsRecording();
}

int Program::Run()
{
    try
    {
        // The facade keeps its keep-alive thread running until Stop(), but does
        // not return until the emulator has finished loading the ROM and
        // starting devices.
        _emulator->Start();
    }
    catch (const std::exception& error)
    {
        _gui->ReportError(std::string("Unable to start emulator: ") + error.what());
        return EXIT_FAILURE;
    }

    try
    {
        // State loads must happen after initialization, otherwise the startup
        // reset would overwrite the replay state and timing.
        ApplyStartupReplayOptions();
    }
    catch (const std::exception& error)
    {
        _gui->ReportError(std::string("Unable to apply startup options: ") + error.what());
    }

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

        const auto progress = _replaySink->GetProgress();

        if (_wasReplayActive && !progress.active)
        {
            _gui->ShowMessage("Replay complete");
        }

        _wasReplayActive = progress.active;
        _gui->SetReplayState(_inputRecorder->IsRecording(), progress);

        const sf::Time elapsed = frameClock.restart();

        _gui->BeginFrame(elapsed);
        _display->Render([this] { _gui->Render(); });

        if (_gui->ConsumeWindowChange())
        {
            RecreateWindow();
        }
    }

    // Release keys before writing the bundle so held keys produce matching KeyUp events.
    ReleaseAllKeys();

    if (_inputRecorder->IsRecording())
    {
        StopReplayRecording();
    }

    _replaySink->Stop();
    _emulator->Stop();
    _display->Close();
    _gui->Shutdown();

    _settings->Save(_settingsPath);

    return EXIT_SUCCESS;
}
