#include "Gui.h"

#include <Compucolor/App/DebugDisassembler.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <system_error>

#include <imgui.h>
#include <imgui-SFML.h>

using Compucolor::Impl::App::Sfml::Gui;

namespace fs = std::filesystem;

namespace
{
    constexpr const char* BrowserPopup = "Select Disk Image";
    constexpr const char* UnsavedPopup = "Unsaved Changes";
    constexpr const char* ErrorPopup = "Error";
    constexpr const char* DiskExtension = ".ccvf";
    constexpr const char* ReplayExtension = ".json";
    constexpr const char* ReplayFileNameSuffix = ".cc2replay.json";
    constexpr const char* StateFileName = "compucolor-state.json";
}


void ApplyCompucolorTheme(ImGuiStyle& style)
{
    using Color = ImVec4;

    // Geometry
    style.WindowPadding = ImVec2(12.f, 10.f);
    style.FramePadding = ImVec2(9.f, 6.f);
    style.CellPadding = ImVec2(8.f, 5.f);
    style.ItemSpacing = ImVec2(8.f, 7.f);
    style.ItemInnerSpacing = ImVec2(6.f, 5.f);
    style.IndentSpacing = 20.f;
    style.ScrollbarSize = 13.f;
    style.GrabMinSize = 10.f;

    // Subtle rounding: industrial equipment, not a mobile app.
    style.WindowRounding = 7.f;
    style.ChildRounding = 5.f;
    style.FrameRounding = 4.f;
    style.PopupRounding = 5.f;
    style.ScrollbarRounding = 6.f;
    style.GrabRounding = 4.f;
    style.TabRounding = 4.f;

    style.WindowBorderSize = 1.f;
    style.ChildBorderSize = 1.f;
    style.PopupBorderSize = 1.f;
    style.FrameBorderSize = 0.f;
    style.TabBorderSize = 0.f;

    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.5f);

    style.SeparatorTextBorderSize = 1.f;
    style.SeparatorTextAlign = ImVec2(0.0f, 0.5f);
    style.SeparatorTextPadding = ImVec2(8.f, 5.f);

    // Palette
    const Color background       = Color(0.067f, 0.110f, 0.145f, 1.00f);
    const Color surface          = Color(0.090f, 0.153f, 0.192f, 1.00f);
    const Color elevated         = Color(0.122f, 0.192f, 0.231f, 1.00f);
    const Color hover            = Color(0.165f, 0.259f, 0.290f, 1.00f);
    const Color border           = Color(0.220f, 0.298f, 0.333f, 1.00f);

    const Color text             = Color(0.914f, 0.898f, 0.835f, 1.00f);
    const Color muted            = Color(0.565f, 0.647f, 0.659f, 1.00f);

    const Color amber            = Color(0.914f, 0.725f, 0.357f, 1.00f);
    const Color amberHover       = Color(0.980f, 0.796f, 0.435f, 1.00f);
    const Color green            = Color(0.467f, 0.788f, 0.690f, 1.00f);

    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text]                  = text;
    colors[ImGuiCol_TextDisabled]          = muted;

    colors[ImGuiCol_WindowBg]              = background;
    colors[ImGuiCol_ChildBg]               = background;
    colors[ImGuiCol_PopupBg]               = surface;

    colors[ImGuiCol_Border]                = border;
    colors[ImGuiCol_BorderShadow]          = Color(0.f, 0.f, 0.f, 0.f);

    colors[ImGuiCol_FrameBg]               = surface;
    colors[ImGuiCol_FrameBgHovered]        = elevated;
    colors[ImGuiCol_FrameBgActive]         = hover;

    colors[ImGuiCol_TitleBg]               = background;
    colors[ImGuiCol_TitleBgActive]         = surface;
    colors[ImGuiCol_TitleBgCollapsed]      = background;

    colors[ImGuiCol_MenuBarBg]              = surface;

    colors[ImGuiCol_ScrollbarBg]           = background;
    colors[ImGuiCol_ScrollbarGrab]         = elevated;
    colors[ImGuiCol_ScrollbarGrabHovered]  = border;
    colors[ImGuiCol_ScrollbarGrabActive]   = amber;

    colors[ImGuiCol_CheckMark]             = green;
    colors[ImGuiCol_SliderGrab]            = amber;
    colors[ImGuiCol_SliderGrabActive]      = amberHover;

    colors[ImGuiCol_Button]                = elevated;
    colors[ImGuiCol_ButtonHovered]         = hover;
    colors[ImGuiCol_ButtonActive]          = amber;

    colors[ImGuiCol_Header]                = elevated;
    colors[ImGuiCol_HeaderHovered]         = hover;
    colors[ImGuiCol_HeaderActive]          = Color(0.24f, 0.34f, 0.35f, 1.f);

    colors[ImGuiCol_Separator]             = border;
    colors[ImGuiCol_SeparatorHovered]      = amber;
    colors[ImGuiCol_SeparatorActive]       = amberHover;

    colors[ImGuiCol_ResizeGrip]            = Color(amber.x, amber.y, amber.z, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]     = amber;
    colors[ImGuiCol_ResizeGripActive]      = amberHover;

    colors[ImGuiCol_Tab]                   = background;
    colors[ImGuiCol_TabHovered]            = hover;
    colors[ImGuiCol_TabActive]             = surface;
    colors[ImGuiCol_TabUnfocused]          = background;
    colors[ImGuiCol_TabUnfocusedActive]    = surface;

    colors[ImGuiCol_PlotLines]             = green;
    colors[ImGuiCol_PlotLinesHovered]      = amber;
    colors[ImGuiCol_PlotHistogram]         = amber;
    colors[ImGuiCol_PlotHistogramHovered]  = amberHover;

    colors[ImGuiCol_TableHeaderBg]         = elevated;
    colors[ImGuiCol_TableBorderStrong]     = border;
    colors[ImGuiCol_TableBorderLight]      = surface;
    colors[ImGuiCol_TableRowBg]            = Color(0.f, 0.f, 0.f, 0.f);
    colors[ImGuiCol_TableRowBgAlt]         = Color(1.f, 1.f, 1.f, 0.025f);

    colors[ImGuiCol_TextSelectedBg]        = Color(
        amber.x, amber.y, amber.z, 0.30f
    );

    colors[ImGuiCol_DragDropTarget]        = amber;

    colors[ImGuiCol_NavHighlight]          = amber;
    colors[ImGuiCol_NavWindowingHighlight] = amber;
    colors[ImGuiCol_NavWindowingDimBg]     = Color(0.f, 0.f, 0.f, 0.55f);
    colors[ImGuiCol_ModalWindowDimBg]       = Color(0.f, 0.f, 0.f, 0.65f);
}


Gui::Gui(
    std::shared_ptr<sf::RenderWindow> window,
    std::shared_ptr<Settings> settings,
    Compucolor::CompucolorEmulatorProcessor& emulator,
    std::shared_ptr<DebugMonitor> debug,
    std::shared_ptr<DebugHub> debugHub,
    std::shared_ptr<Display> display,
    std::shared_ptr<AudioOutput> audio
):
    _window(std::move(window)),
    _settings(std::move(settings)),
    _emulator(emulator),
    _debug(std::move(debug)),
    _debugHub(std::move(debugHub)),
    _display(std::move(display)),
    _audio(std::move(audio)),
    _debugRecording(_debugHub && _debugHub->IsEnabled())
{
}

bool Gui::Init()
{
    _ready = ImGui::SFML::Init(*_window);
    _uiScale = 0.f;

    if (_ready)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
    }
    else
    {
        std::cerr << "Unable to initialize the GUI overlay\n";
    }

    return _ready;
}

void Gui::Shutdown()
{
    if (_ready)
    {
        // Without this the window keeps a freed X cursor and recreating it raises BadCursor.
        if (!_arrowCursor)
        {
            _arrowCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);
        }

        if (_arrowCursor)
        {
            _window->setMouseCursor(*_arrowCursor);
        }

        ImGui::SFML::Shutdown();
        _ready = false;
    }
}

void Gui::SetQuitHandler(std::function<void()> handler)
{
    _quit = std::move(handler);
}

void Gui::SetReplayHandlers(
    std::function<void(const std::string&)> startRecording,
    std::function<void()> stopRecording
)
{
    _startReplayRecording = std::move(startRecording);
    _stopReplayRecording = std::move(stopRecording);
}

void Gui::SetReplayState(
    const bool recording,
    const Compucolor::Impl::App::Replay::ReplayProgress& progress
)
{
    _replayRecording = recording;
    _replayProgress = progress;
}

void Gui::ReportError(const std::string& message)
{
    _errorMessage = message;
}

void Gui::ShowMessage(const std::string& message)
{
    ShowStatus(message);
}

void Gui::ApplyStartupSettings()
{
    ApplyMachineSettings();
    ApplySoundSettings();

    for (int drive = 0; drive < 2; drive++)
    {
        const std::string path = _settings->GetString(DiskSettingKey(drive));

        if (!path.empty())
        {
            MountDisk(drive, path);
        }
    }
}

void Gui::ApplyMachineSettings()
{
    _emulator.SetLowercaseCharset(_settings->GetString("charset") == "lowercase");
    _emulator.SetSpeedRegulated(_settings->GetBool("speed_regulated"));

    if (_settings->GetString("rom_version") == "v6.78")
    {
        _emulator.SetRomVersion(Compucolor::RomVersion::V6_78);
    }
}

void Gui::ApplySoundSettings()
{
    if (!_audio)
    {
        return;
    }

    const int volume = std::clamp(_settings->GetInt("sound_volume"), 0, 100);
    _audio->SetVolume(static_cast<float>(volume) / 100.0f);
    _audio->SetEnabled(_settings->GetBool("sound_enabled"));
}

void Gui::SetSoundEnabled(const bool enabled)
{
    _settings->SetBool("sound_enabled", enabled);

    if (_audio)
    {
        _audio->SetEnabled(enabled);
    }

    ShowStatus(enabled ? "Sound enabled" : "Sound muted");
}

void Gui::SetSoundVolume(const int volume)
{
    const int clamped = std::clamp(volume, 0, 100);
    _settings->SetInt("sound_volume", clamped);

    if (_audio)
    {
        _audio->SetVolume(static_cast<float>(clamped) / 100.0f);
    }
}

void Gui::ToggleSound()
{
    SetSoundEnabled(!_settings->GetBool("sound_enabled"));
}

void Gui::SaveState()
{
    try
    {
        _emulator.SaveState();
        ShowStatus("Saved emulator state");
    }
    catch (const std::exception& error)
    {
        _errorMessage = std::string("Unable to save state: ") + error.what();
    }
}

void Gui::LoadState()
{
    try
    {
        _emulator.LoadState();
        ShowStatus("Loaded emulator state");
    }
    catch (const std::exception& error)
    {
        _errorMessage = std::string("Unable to load state: ") + error.what();
    }
}

void Gui::ProcessEvent(const sf::Event& event)
{
    if (_ready)
    {
        ImGui::SFML::ProcessEvent(*_window, event);
    }
}

bool Gui::WantsKeyboard() const
{
    return _ready && ImGui::GetIO().WantCaptureKeyboard;
}

void Gui::UpdateScale()
{
    // Quarter steps avoid rebuilding the style on every pixel of a resize.
    const float height = static_cast<float>(_window->getSize().y);
    const float scale = std::clamp(
        std::round(height / 360.f * 4.f) / 4.f,
        1.f,
        4.f
    );

    if (scale == _uiScale)
    {
        return;
    }

    _uiScale = scale;

    ImGuiStyle style;
    ApplyCompucolorTheme(style);

    style.ScaleAllSizes(scale);
    ImGui::GetStyle() = style;

    ImGui::GetIO().FontGlobalScale = scale;
}

void Gui::BeginFrame(const sf::Time elapsed)
{
    if (!_ready)
    {
        return;
    }

    UpdateScale();
    ImGui::SFML::Update(*_window, elapsed);

    DrawMenu();

    if (_showDiskPanel && _menuVisible)
    {
        DrawDiskPanel();
    }

    DrawSettings();
    DrawKeyHelp();
    DrawAbout();
    DrawDebugWindows();

    if (_openBrowser)
    {
        ImGui::OpenPopup(BrowserPopup);
        _openBrowser = false;
    }

    if (_openUnsaved)
    {
        ImGui::OpenPopup(UnsavedPopup);
        _openUnsaved = false;
    }

    if (!_errorMessage.empty() && !ImGui::IsPopupOpen(ErrorPopup))
    {
        ImGui::OpenPopup(ErrorPopup);
    }

    DrawBrowser();
    DrawUnsavedPrompt();
    DrawError();
    DrawStatus();
}

void Gui::ShowStatus(const std::string& message)
{
    _status = message;
    _statusUntil = std::chrono::steady_clock::now() + std::chrono::seconds(3);
}

void Gui::DrawStatus()
{
    if (_status.empty() || std::chrono::steady_clock::now() >= _statusUntil)
    {
        return;
    }

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::SetNextWindowPos(
        ImVec2(8.f * _uiScale, ImGui::GetIO().DisplaySize.y - 8.f * _uiScale),
        ImGuiCond_Always,
        ImVec2(0.f, 1.f)
    );
    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f * _uiScale, 6.f * _uiScale));

    if (ImGui::Begin("##status", nullptr, flags))
    {
        ImGui::TextUnformatted(_status.c_str());
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void Gui::DrawMenuStatus()
{
    struct Chip {
        std::string text;
        ImVec4 color;
    };

    std::vector<Chip> chips;

    if (_emulator.IsPaused())
    {
        chips.push_back({"PAUSED", ImVec4(1.0f, 0.78f, 0.25f, 1.0f)});
    }

    if (!_emulator.IsSpeedRegulated())
    {
        chips.push_back({"UNREGULATED", ImVec4(0.40f, 0.85f, 1.0f, 1.0f)});
    }

    if (!_settings->GetBool("sound_enabled"))
    {
        chips.push_back({"SOUND OFF", ImVec4(0.75f, 0.75f, 0.82f, 1.0f)});
    }

    if (_replayRecording)
    {
        chips.push_back({"REC", ImVec4(1.0f, 0.42f, 0.42f, 1.0f)});
    }

    if (_replayProgress.active)
    {
        chips.push_back({"REPLAY", ImVec4(0.45f, 0.95f, 0.65f, 1.0f)});
    }

    for (int drive = 0; drive < 2; drive++)
    {
        if (IsModified(drive))
        {
            chips.push_back({std::string("DISK ") + DriveLabel(drive) + " *", ImVec4(1.0f, 0.55f, 0.45f, 1.0f)});
        }
    }

    if (chips.empty())
    {
        return;
    }

    const float spacing = 10.f * _uiScale;
    float totalWidth = 0.f;

    for (const Chip& chip : chips)
    {
        totalWidth += ImGui::CalcTextSize(chip.text.c_str()).x + spacing;
    }

    // Leave the menu names enough room on small windows.
    if (ImGui::GetWindowWidth() - totalWidth < 430.f * _uiScale)
    {
        return;
    }

    float x = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x;
    const float y = ImGui::GetCursorPosY();

    for (auto it = chips.rbegin(); it != chips.rend(); ++it)
    {
        x -= ImGui::CalcTextSize(it->text.c_str()).x + spacing;
        ImGui::SetCursorPos(ImVec2(x, y));
        ImGui::TextColored(it->color, "%s", it->text.c_str());
    }
}

void Gui::SaveScreenshot()
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);

    std::ostringstream name;
    name << "cc2-" << std::put_time(&local, "%Y%m%d-%H%M%S") << ".png";

    const fs::path directory = fs::path(Settings::DefaultPath()).parent_path() / "screenshots";

    std::error_code error;
    fs::create_directories(directory, error);

    const fs::path target = directory / name.str();

    if (_display->SaveScreenshot(target.string()))
    {
        ShowStatus("Saved " + target.string());
    }
    else
    {
        _errorMessage = "Unable to save the screenshot to " + target.string();
    }
}

void Gui::Render()
{
    if (_ready)
    {
        ImGui::SFML::Render(*_window);
    }
}

void Gui::ToggleMenu()
{
    _menuVisible = !_menuVisible;

    if (!_menuVisible)
    {
        ShowStatus("Menu hidden - press Alt+M to show it again");
    }
}

bool Gui::ConsumeWindowChange()
{
    const bool changed = _windowChanged;
    _windowChanged = false;

    return changed;
}

void Gui::RequestQuit()
{
    GuardUnsaved({0, 1}, [this] {
        if (_quit)
        {
            _quit();
        }
    });
}

void Gui::DrawMenu()
{
    if (!_menuVisible || !ImGui::BeginMainMenuBar())
    {
        return;
    }

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save State"))
        {
            SaveState();
        }

        std::error_code stateError;
        const bool hasState = fs::exists(StateFileName, stateError) && !stateError;
        if (ImGui::MenuItem("Load State", nullptr, false, hasState))
        {
            LoadState();
        }

        ImGui::Separator();

        if (ImGui::MenuItem(
                "Start Replay Recording...",
                nullptr,
                false,
                !_replayRecording && static_cast<bool>(_startReplayRecording)
            ))
        {
            OpenBrowser(BrowserMode::SaveReplay, 0);
        }

        if (ImGui::MenuItem(
                "Stop Replay Recording",
                nullptr,
                false,
                _replayRecording && static_cast<bool>(_stopReplayRecording)
            ))
        {
            _stopReplayRecording();
        }

        ImGui::Separator();

        for (int drive = 0; drive < 2; drive++)
        {
            ImGui::PushID(drive);

            const std::string mountLabel = std::string("Mount Disk ") + DriveLabel(drive) + "...";
            if (ImGui::MenuItem(mountLabel.c_str()))
            {
                GuardUnsaved({drive}, [this, drive] { OpenBrowser(BrowserMode::Mount, drive); });
            }

            const auto floppy = _emulator.GetFloppy(drive);
            const bool mounted = floppy && floppy->IsMounted();

            const std::string ejectLabel = std::string("Eject Disk ") + DriveLabel(drive);
            if (ImGui::MenuItem(ejectLabel.c_str(), nullptr, false, mounted))
            {
                GuardUnsaved({drive}, [this, drive] { EjectDisk(drive); });
            }

            const std::string saveLabel = std::string("Save Disk ") + DriveLabel(drive);
            if (ImGui::MenuItem(saveLabel.c_str(), nullptr, false, mounted))
            {
                SaveDisk(drive);
            }

            const std::string saveAsLabel = std::string("Save Disk ") + DriveLabel(drive) + " As...";
            if (ImGui::MenuItem(saveAsLabel.c_str(), nullptr, false, mounted))
            {
                OpenBrowser(BrowserMode::SaveAs, drive);
            }

            ImGui::PopID();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save Screenshot", "Alt+P"))
        {
            SaveScreenshot();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit"))
        {
            RequestQuit();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Machine"))
    {
        if (ImGui::MenuItem("Reset"))
        {
            _emulator.Reset(false);
        }

        if (ImGui::MenuItem("Hard Reset"))
        {
            _emulator.Reset(true);
        }

        ImGui::Separator();

        bool paused = _emulator.IsPaused();
        if (ImGui::MenuItem("Pause", nullptr, &paused))
        {
            if (paused)
            {
                _emulator.Pause();
            }
            else
            {
                _emulator.Resume();
            }
        }

        bool regulated = _emulator.IsSpeedRegulated();
        if (ImGui::MenuItem("Real Speed", nullptr, &regulated))
        {
            _emulator.SetSpeedRegulated(regulated);
            _settings->SetBool("speed_regulated", regulated);
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Debug"))
    {
        if (ImGui::MenuItem("Record trace", nullptr, &_debugRecording))
        {
            if (_debugHub)
            {
                _debugHub->SetEnabled(_debugRecording);
            }

            if (_debug)
            {
                _debug->SetEnabled(_debugRecording);
            }
        }

        ImGui::Separator();

        ImGui::MenuItem("CPU", nullptr, &_showCpuDebug);
        ImGui::MenuItem("Memory", nullptr, &_showMemoryDebug);
        ImGui::MenuItem("Ports", nullptr, &_showPortDebug);
        ImGui::MenuItem("Trace", nullptr, &_showTraceDebug);
        ImGui::MenuItem("TMS5501", nullptr, &_showTms5501Debug);
        ImGui::MenuItem("SMC5027", nullptr, &_showSmc5027Debug);
        ImGui::MenuItem("Replay Status", nullptr, &_showReplayStatus);

        ImGui::Separator();

        if (ImGui::MenuItem("Clear traces") && _debug)
        {
            _debug->Clear();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Sound"))
    {
        bool enabled = _settings->GetBool("sound_enabled");
        if (ImGui::MenuItem("Enable Sound", "Alt+A", &enabled))
        {
            SetSoundEnabled(enabled);
        }

        ImGui::Separator();

        int volume = std::clamp(_settings->GetInt("sound_volume"), 0, 100);
        ImGui::SetNextItemWidth(140.f * _uiScale);
        if (ImGui::SliderInt("Volume", &volume, 0, 100, "%d%%"))
        {
            SetSoundVolume(volume);
        }

        ImGui::TextDisabled("SoundWare 1-bit audio");
        ImGui::SameLine();
        HelpMarker("Some disks use the SoundWare add-on. Sound is generated by toggling the serial line, so it only affects programs written for it.");

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View"))
    {
        const int scale = _settings->GetInt("window_scale");

        for (int option = 1; option <= 4; option++)
        {
            const std::string label = std::to_string(option) + "x";
            const std::string shortcut = "Alt+" + std::to_string(option);

            if (ImGui::MenuItem(label.c_str(), shortcut.c_str(), scale == option && !_settings->GetBool("fullscreen")))
            {
                SetScale(option);
            }
        }

        bool fullscreen = _settings->GetBool("fullscreen");
        if (ImGui::MenuItem("Fullscreen", "Alt+Enter", &fullscreen))
        {
            SetFullscreen(fullscreen);
        }

        bool smooth = _settings->GetBool("smooth_scaling");
        if (ImGui::MenuItem("Smooth Scaling", "Alt+S", &smooth))
        {
            SetSmooth(smooth);
        }

        ImGui::Separator();

        ImGui::MenuItem("Disk Panel", nullptr, &_showDiskPanel);

        if (ImGui::MenuItem("Hide Menu", "Alt+M"))
        {
            _menuVisible = false;
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Settings"))
    {
        ImGui::MenuItem("Preferences...", nullptr, &_showSettings);

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help"))
    {
        ImGui::MenuItem("Keyboard", nullptr, &_showKeys);
        ImGui::MenuItem("About", nullptr, &_showAbout);

        ImGui::EndMenu();
    }

    DrawMenuStatus();

    ImGui::EndMainMenuBar();
}

void Gui::DrawDriveControls(const int drive)
{
    const auto floppy = _emulator.GetFloppy(drive);

    if (!floppy)
    {
        return;
    }

    ImGui::PushID(drive);

    ImGui::Text("Drive %s:", DriveLabel(drive));
    ImGui::SameLine();

    if (floppy->IsMounted())
    {
        const std::string name = fs::path(floppy->GetMountedPath()).filename().string();
        ImGui::Text("%s%s", name.c_str(), floppy->IsModified() ? " *" : "");
    }
    else
    {
        ImGui::TextDisabled("(empty)");
    }

    if (ImGui::Button("Mount..."))
    {
        GuardUnsaved({drive}, [this, drive] { OpenBrowser(BrowserMode::Mount, drive); });
    }

    ImGui::SameLine();

    ImGui::BeginDisabled(!floppy->IsMounted());

    if (ImGui::Button("Eject"))
    {
        GuardUnsaved({drive}, [this, drive] { EjectDisk(drive); });
    }

    ImGui::SameLine();

    if (ImGui::Button("Save"))
    {
        SaveDisk(drive);
    }

    ImGui::EndDisabled();

    ImGui::PopID();
}

void Gui::DrawDiskPanel()
{
    const float menuHeight = ImGui::GetFrameHeight();
    const float width = ImGui::GetIO().DisplaySize.x;

    ImGui::SetNextWindowPos(ImVec2(width - 8.f * _uiScale, menuHeight + 8.f * _uiScale), ImGuiCond_FirstUseEver, ImVec2(1.f, 0.f));

    if (ImGui::Begin("Disks", &_showDiskPanel, ImGuiWindowFlags_AlwaysAutoResize))
    {
        DrawDriveControls(0);
        ImGui::Separator();
        DrawDriveControls(1);
    }

    ImGui::End();
}

void Gui::DrawSettings()
{
    if (!_showSettings)
    {
        return;
    }

    if (ImGui::Begin("Preferences", &_showSettings, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::SeparatorText("Display");

        bool fullscreen = _settings->GetBool("fullscreen");
        if (ImGui::Checkbox("Fullscreen", &fullscreen))
        {
            SetFullscreen(fullscreen);
        }

        int scale = std::clamp(_settings->GetInt("window_scale"), 1, 8);
        ImGui::SetNextItemWidth(200.f * _uiScale);
        if (ImGui::SliderInt("Window scale", &scale, 1, 8))
        {
            SetScale(scale);
        }

        bool smooth = _settings->GetBool("smooth_scaling");
        if (ImGui::Checkbox("Smooth scaling", &smooth))
        {
            SetSmooth(smooth);
        }

        ImGui::SameLine();
        HelpMarker("Smooths the 384x256 display when it is scaled. Best at non-integer window sizes.");

        int theme = static_cast<int>(ThemeFromInt(_settings->GetInt("theme")));
        ImGui::SetNextItemWidth(200.f * _uiScale);
        if (ImGui::Combo(
            "Theme",
            &theme,
            ThemeNames.data(),
            static_cast<int>(ThemeCount())
        ))
        {
            _settings->SetInt("theme", theme);
            _display->SetTheme(ThemeFromInt(theme));
        }


        ImGui::SeparatorText("Sound");

        bool enabled = _settings->GetBool("sound_enabled");
        if (ImGui::Checkbox("Enable sound", &enabled))
        {
            SetSoundEnabled(enabled);
        }

        ImGui::SameLine();
        HelpMarker("Sound comes from the optional SoundWare add-on, not the stock machine.");

        int volume = std::clamp(_settings->GetInt("sound_volume"), 0, 100);
        ImGui::BeginDisabled(!enabled);
        ImGui::SetNextItemWidth(200.f * _uiScale);
        if (ImGui::SliderInt("Volume", &volume, 0, 100, "%d%%"))
        {
            SetSoundVolume(volume);
        }
        ImGui::EndDisabled();

        ImGui::TextDisabled("Only SoundWare-aware disks produce audio.");

        ImGui::SeparatorText("Machine");

        int charset = _settings->GetString("charset") == "lowercase" ? 1 : 0;
        const char* charsets[] = {"Standard", "Lowercase"};
        ImGui::SetNextItemWidth(200.f * _uiScale);
        if (ImGui::Combo("Character set", &charset, charsets, 2))
        {
            _settings->SetString("charset", charset == 1 ? "lowercase" : "standard");
            _emulator.SetLowercaseCharset(charset == 1);
        }

        int rom = _settings->GetString("rom_version") == "v6.78" ? 0 : 1;
        const char* roms[] = {"v6.78", "v8.79"};
        ImGui::SetNextItemWidth(200.f * _uiScale);
        if (ImGui::Combo("System ROM", &rom, roms, 2))
        {
            _settings->SetString("rom_version", roms[rom]);
            _emulator.SetRomVersion(rom == 0 ? Compucolor::RomVersion::V6_78 : Compucolor::RomVersion::V8_79);
        }

        ImGui::TextDisabled("Changing the ROM performs a hard reset.");

        bool regulated = _emulator.IsSpeedRegulated();
        if (ImGui::Checkbox("Run at real speed", &regulated))
        {
            _emulator.SetSpeedRegulated(regulated);
            _settings->SetBool("speed_regulated", regulated);
        }

        ImGui::SameLine();
        HelpMarker("Turn off to run the CPU as fast as possible.");
    }

    ImGui::End();
}

void Gui::DrawKeyHelp()
{
    if (!_showKeys)
    {
        return;
    }

    if (ImGui::Begin("Keyboard", &_showKeys, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::SeparatorText("Emulator hotkeys");
        ImGui::BulletText("Alt+Enter: fullscreen");
        ImGui::BulletText("Alt+1 .. Alt+4: window scale");
        ImGui::BulletText("Alt+S: smooth scaling");
        ImGui::BulletText("Alt+A: sound on / off");
        ImGui::BulletText("Alt+M: show or hide the menu");
        ImGui::BulletText("Alt+P: save a screenshot");
        ImGui::TextDisabled("Save and load state are in the File menu.");

        ImGui::SeparatorText("PC keys on the Compucolor");
        ImGui::BulletText("Alt / right Windows key: REPEAT; left Windows key: AUTO");
        ImGui::BulletText("Ctrl: CONTROL");
        ImGui::BulletText("Symbols follow the printed character, e.g. Shift+; is ':' and Shift+7 is '&'");
        ImGui::BulletText("Arrow keys, Home, Tab: cursor keys");
        ImGui::BulletText("Insert / Delete: insert and delete character");
        ImGui::BulletText("Page Up / Page Down: erase page / erase line");
        ImGui::BulletText("Pause: BREAK; End: HOME");
        ImGui::BulletText("Backspace: left arrow / delete");
        ImGui::BulletText("Enter: RETURN");
        ImGui::BulletText("F1 .. F15: function keys");
    }

    ImGui::End();
}

void Gui::DrawAbout()
{
    if (!_showAbout)
    {
        return;
    }

    if (ImGui::Begin("About", &_showAbout, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Compucolor II Emulator");
        ImGui::TextDisabled("An 8080-based home computer, emulated in C++.");

        ImGui::Separator();

        ImGui::BulletText("Intel 8080 CPU with cycle-paced timing");
        ImGui::BulletText("CRT controller with scroll, blink and plot characters");
        ImGui::BulletText("Two floppy drives with .ccvf disk images");
        ImGui::BulletText("Optional SoundWare add-on audio");

        ImGui::Separator();

        ImGui::TextDisabled("Press Alt+M to show or hide the menu.");
    }

    ImGui::End();
}

void Gui::DrawDebugWindows()
{
    if (_debug)
    {
        // UI-only ring buffers are expensive at instruction frequency. Only
        // fill them while the windows that read them are visible.
        _debug->SetCaptureEnabled(_showTraceDebug || _showPortDebug);
    }

    DrawCpuDebug();
    DrawMemoryDebug();
    DrawPortDebug();
    DrawTraceDebug();
    DrawTms5501Debug();
    DrawSmc5027Debug();
    DrawReplayStatus();
}

void Gui::DrawCpuDebug()
{
    if (!_showCpuDebug || !_debug)
    {
        return;
    }

    if (ImGui::Begin("CPU - Debug", &_showCpuDebug, ImGuiWindowFlags_AlwaysAutoResize))
    {
        const auto cpu = _debug->GetCpuState();

        ImGui::SeparatorText("State");
        ImGui::Text(
            "PC %04X    SP %04X    CYC %llu",
            cpu.pc,
            cpu.sp,
            static_cast<unsigned long long>(cpu.cycles)
        );
        ImGui::Text(
            "Emulated time %llu us",
            static_cast<unsigned long long>(cpu.emulatedNanoseconds / 1000)
        );
        ImGui::Text(
            "Interrupt %s    RST %02X    IFF %s",
            cpu.interruptPending ? "pending" : "none",
            static_cast<unsigned int>(cpu.interruptVector),
            cpu.iff ? "set" : "clear"
        );

        ImGui::SeparatorText("Registers");
        ImGui::Text(
            "A %02X   B %02X   C %02X   D %02X",
            static_cast<unsigned int>(cpu.a),
            static_cast<unsigned int>(cpu.b),
            static_cast<unsigned int>(cpu.c),
            static_cast<unsigned int>(cpu.d)
        );
        ImGui::Text(
            "E %02X   H %02X   L %02X   HL %04X",
            static_cast<unsigned int>(cpu.e),
            static_cast<unsigned int>(cpu.h),
            static_cast<unsigned int>(cpu.l),
            static_cast<unsigned int>(cpu.h) << 8 | static_cast<unsigned int>(cpu.l)
        );

        ImGui::SeparatorText("Flags");
        struct Flag {
            const char* name;
            bool value;
        };

        const Flag flags[] = {
            {"S", cpu.sf}, {"Z", cpu.zf}, {"H", cpu.hf}, {"P", cpu.pf},
            {"C", cpu.cf}, {"I", cpu.iff}, {"HALT", cpu.halted}
        };

        for (std::size_t i = 0; i < std::size(flags); i++)
        {
            if (i != 0)
            {
                ImGui::SameLine();
            }

            ImGui::TextColored(
                flags[i].value
                    ? ImVec4(0.98f, 0.80f, 0.44f, 1.f)
                    : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled),
                "%s",
                flags[i].name
            );
        }

        ImGui::SeparatorText("Next instruction");
        std::array<uint8_t, 3> bytes = {
            _debug->ReadMemory(cpu.pc),
            _debug->ReadMemory(static_cast<uint16_t>(cpu.pc + 1)),
            _debug->ReadMemory(static_cast<uint16_t>(cpu.pc + 2))
        };

        const auto instruction = Disassemble8080(bytes.data(), cpu.pc);

        ImGui::Text("%04X: %s", cpu.pc, instruction.text.c_str());
        ImGui::TextDisabled(
            "Bytes: %02X %02X %02X",
            static_cast<unsigned int>(bytes[0]),
            static_cast<unsigned int>(bytes[1]),
            static_cast<unsigned int>(bytes[2])
        );

        ImGui::SeparatorText("Control");
        const bool paused = _emulator.IsPaused();

        if (ImGui::Button(paused ? "Resume" : "Pause"))
        {
            if (paused)
            {
                _emulator.Resume();
            }
            else
            {
                _emulator.Pause();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            _emulator.Reset(false);
        }

        ImGui::SameLine();
        if (ImGui::Button("Hard Reset"))
        {
            _emulator.Reset(true);
        }

        ImGui::TextDisabled("Pause the machine before editing memory.");
    }

    ImGui::End();
}

void Gui::DrawMemoryDebug()
{
    if (!_showMemoryDebug || !_debug)
    {
        return;
    }

    ImGui::SetNextWindowSize(
        ImVec2(520.f * _uiScale, 340.f * _uiScale),
        ImGuiCond_FirstUseEver
    );

    if (ImGui::Begin("Memory - Debug", &_showMemoryDebug))
    {
        ImGui::SetNextItemWidth(90.f * _uiScale);

        bool gotoAddress = ImGui::InputText(
            "Address",
            _memoryAddressText,
            sizeof(_memoryAddressText),
            ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue
        );

        ImGui::SameLine();
        gotoAddress = ImGui::Button("Goto") || gotoAddress;

        if (gotoAddress)
        {
            uint16_t parsed = 0;
            if (ParseHexAddress(_memoryAddressText, parsed))
            {
                _memoryViewAddress = static_cast<uint16_t>(parsed & 0xFFC0);
                std::snprintf(
                    _memoryAddressText,
                    sizeof(_memoryAddressText),
                    "%04X",
                    _memoryViewAddress
                );
            }
            else
            {
                ShowStatus("Enter a hexadecimal address (0000-FFFF)");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("-64"))
        {
            _memoryViewAddress = static_cast<uint16_t>(_memoryViewAddress - 64);
        }

        ImGui::SameLine();
        if (ImGui::Button("+64"))
        {
            _memoryViewAddress = static_cast<uint16_t>(_memoryViewAddress + 64);
        }

        ImGui::TextDisabled(
            "Region: %s%s",
            MemoryRegion(_memoryViewAddress),
            _emulator.IsPaused() ? "" : "  -  running (edits disabled)"
        );

        ImGui::BeginDisabled(!_emulator.IsPaused());

        if (ImGui::BeginTable(
            "debug-memory",
            10,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_Hideable
        ))
        {
            ImGui::TableSetupColumn("Addr", ImGuiTableColumnFlags_WidthFixed, 48.f * _uiScale);

            for (int column = 0; column < 8; column++)
            {
                char label[8];
                std::snprintf(label, sizeof(label), "%X", column);
                ImGui::TableSetupColumn(label, ImGuiTableColumnFlags_WidthFixed, 32.f * _uiScale);
            }

            ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, 110.f * _uiScale);
            ImGui::TableHeadersRow();

            constexpr int RowCount = 8;

            for (int row = 0; row < RowCount; row++)
            {
                const uint16_t rowAddress = static_cast<uint16_t>(
                    _memoryViewAddress + row * 8
                );

                std::array<char, 9> ascii{};
                ascii[8] = '\0';

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%04X", rowAddress);

                for (int column = 0; column < 8; column++)
                {
                    const uint16_t address = static_cast<uint16_t>(rowAddress + column);
                    uint8_t byte = _debug->ReadMemory(address);

                    ImGui::TableNextColumn();
                    ImGui::PushID(static_cast<int>(address));

                    // Tighter padding lets both hex digits fit without widening
                    // the table or the window.
                    ImGui::PushStyleVar(
                        ImGuiStyleVar_FramePadding,
                        ImVec2(2.f * _uiScale, 2.f * _uiScale)
                    );
                    ImGui::SetNextItemWidth(-1.f);

                    if (ImGui::InputScalar(
                        "##byte",
                        ImGuiDataType_U8,
                        &byte,
                        nullptr,
                        nullptr,
                        "%02X",
                        ImGuiInputTextFlags_CharsHexadecimal
                    ))
                    {
                        _debug->WriteMemory(address, byte);
                    }

                    ImGui::PopStyleVar();
                    ImGui::PopID();

                    ascii[static_cast<std::size_t>(column)] =
                        byte >= 0x20 && byte <= 0x7E
                            ? static_cast<char>(byte)
                            : '.';
                }

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(ascii.data());
            }

            ImGui::EndTable();
        }

        ImGui::EndDisabled();
    }

    ImGui::End();
}

void Gui::DrawPortDebug()
{
    if (!_showPortDebug || !_debug)
    {
        return;
    }

    ImGui::SetNextWindowSize(
        ImVec2(660.f * _uiScale, 420.f * _uiScale),
        ImGuiCond_FirstUseEver
    );

    if (ImGui::Begin("Ports - Debug", &_showPortDebug))
    {
        if (ImGui::Checkbox("Record trace", &_debugRecording))
        {
            if (_debugHub)
            {
                _debugHub->SetEnabled(_debugRecording);
            }

            if (_debug)
            {
                _debug->SetEnabled(_debugRecording);
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Clear"))
        {
            _debug->Clear();
        }

        if (ImGui::CollapsingHeader("Filters"))
        {
            ImGui::Checkbox("Enabled", &_portFilter.enabled);
            ImGui::SameLine();
            ImGui::Checkbox("Stop on match", &_portFilter.stopOnMatch);

            ImGui::Checkbox("Port range", &_portFilter.filterByPort);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(60.f * _uiScale);
            ImGui::InputScalar(
                "From##port-from",
                ImGuiDataType_U8,
                &_portFilter.portFrom,
                nullptr,
                nullptr,
                "%02X",
                ImGuiInputTextFlags_CharsHexadecimal
            );
            ImGui::SameLine();
            ImGui::SetNextItemWidth(60.f * _uiScale);
            ImGui::InputScalar(
                "To##port-to",
                ImGuiDataType_U8,
                &_portFilter.portTo,
                nullptr,
                nullptr,
                "%02X",
                ImGuiInputTextFlags_CharsHexadecimal
            );

            int direction = static_cast<int>(_portFilter.direction);
            ImGui::RadioButton("Any##port-direction", &direction, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Read##port-direction", &direction, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Write##port-direction", &direction, 2);
            _portFilter.direction = static_cast<DebugPortDirection>(direction);

            if (ImGui::Button("Rearm##ports"))
            {
                _debug->RearmTriggers();
            }

            ImGui::SameLine();
            ImGui::Text(
                "Triggered: %s",
                _debug->IsPortTriggered() ? "yes" : "no"
            );
        }

        _debug->SetPortFilter(_portFilter);

        ImGui::TextDisabled("Newest first - %d entries buffered", 512);

        if (ImGui::BeginTable(
            "debug-ports",
            7,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX
        ))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 60.f * _uiScale);
            ImGui::TableSetupColumn("PC", ImGuiTableColumnFlags_WidthFixed, 60.f * _uiScale);
            ImGui::TableSetupColumn("R/W", ImGuiTableColumnFlags_WidthFixed, 45.f * _uiScale);
            ImGui::TableSetupColumn("Port", ImGuiTableColumnFlags_WidthFixed, 55.f * _uiScale);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 65.f * _uiScale);
            ImGui::TableSetupColumn("Bits", ImGuiTableColumnFlags_WidthFixed, 95.f * _uiScale);
            ImGui::TableSetupColumn("Device");
            ImGui::TableHeadersRow();

            const auto ports = _debug->GetPorts();

            for (auto it = ports.rbegin(); it != ports.rend(); ++it)
            {
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%llu", static_cast<unsigned long long>(it->sequence));

                ImGui::TableNextColumn();
                ImGui::Text("%04X", it->pc);

                ImGui::TableNextColumn();
                ImGui::TextColored(
                    it->isWrite
                        ? ImVec4(0.98f, 0.80f, 0.44f, 1.f)
                        : ImVec4(0.47f, 0.79f, 0.69f, 1.f),
                    "%s",
                    it->isWrite ? "OUT" : "IN"
                );

                ImGui::TableNextColumn();
                ImGui::Text("%02X", static_cast<unsigned int>(it->port));

                ImGui::TableNextColumn();
                ImGui::Text("%02X", static_cast<unsigned int>(it->value));

                ImGui::TableNextColumn();
                const std::string bits = ByteBits(it->value);
                ImGui::TextUnformatted(bits.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", PortDevice(it->port));
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();
}

void Gui::DrawTraceDebug()
{
    if (!_showTraceDebug || !_debug)
    {
        return;
    }

    ImGui::SetNextWindowSize(
        ImVec2(760.f * _uiScale, 460.f * _uiScale),
        ImGuiCond_FirstUseEver
    );

    if (ImGui::Begin("Trace - Debug", &_showTraceDebug))
    {
        if (ImGui::Checkbox("Record trace", &_debugRecording))
        {
            if (_debugHub)
            {
                _debugHub->SetEnabled(_debugRecording);
            }

            if (_debug)
            {
                _debug->SetEnabled(_debugRecording);
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Clear"))
        {
            _debug->Clear();
        }

        if (ImGui::CollapsingHeader("Filters"))
        {
            ImGui::Checkbox("Enabled", &_instructionFilter.enabled);
            ImGui::SameLine();
            ImGui::Checkbox("Stop on match", &_instructionFilter.stopOnMatch);

            ImGui::Checkbox("PC range", &_instructionFilter.filterByPc);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70.f * _uiScale);
            ImGui::InputScalar(
                "From##pc-from",
                ImGuiDataType_U16,
                &_instructionFilter.pcFrom,
                nullptr,
                nullptr,
                "%04X",
                ImGuiInputTextFlags_CharsHexadecimal
            );
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70.f * _uiScale);
            ImGui::InputScalar(
                "To##pc-to",
                ImGuiDataType_U16,
                &_instructionFilter.pcTo,
                nullptr,
                nullptr,
                "%04X",
                ImGuiInputTextFlags_CharsHexadecimal
            );

            ImGui::SetNextItemWidth(220.f * _uiScale);
            ImGui::InputText(
                "Mnemonic contains",
                _instructionFilterText,
                sizeof(_instructionFilterText),
                ImGuiInputTextFlags_CharsUppercase
            );
            _instructionFilter.text = _instructionFilterText;

            if (ImGui::Button("Rearm##trace"))
            {
                _debug->RearmTriggers();
            }

            ImGui::SameLine();
            ImGui::Text(
                "Triggered: %s",
                _debug->IsInstructionTriggered() ? "yes" : "no"
            );
        }

        _debug->SetInstructionFilter(_instructionFilter);

        ImGui::TextDisabled("Newest first - shows the last 512 instructions");

        if (ImGui::BeginTable(
            "debug-trace",
            6,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX
        ))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 60.f * _uiScale);
            ImGui::TableSetupColumn("PC", ImGuiTableColumnFlags_WidthFixed, 60.f * _uiScale);
            ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed, 95.f * _uiScale);
            ImGui::TableSetupColumn("Instruction");
            ImGui::TableSetupColumn("Cycles", ImGuiTableColumnFlags_WidthFixed, 60.f * _uiScale);
            ImGui::TableSetupColumn("Flags", ImGuiTableColumnFlags_WidthFixed, 95.f * _uiScale);
            ImGui::TableHeadersRow();

            const auto instructions = _debug->GetInstructions();

            for (auto it = instructions.rbegin(); it != instructions.rend(); ++it)
            {
                const auto instruction = Disassemble8080(
                    it->bytes.data(),
                    it->before.pc
                );

                std::string flags;
                flags += it->before.sf ? 'S' : '-';
                flags += it->before.zf ? 'Z' : '-';
                flags += it->before.hf ? 'H' : '-';
                flags += it->before.pf ? 'P' : '-';
                flags += it->before.cf ? 'C' : '-';
                flags += it->before.iff ? 'I' : '-';

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%llu", static_cast<unsigned long long>(it->sequence));

                ImGui::TableNextColumn();
                ImGui::Text("%04X", it->before.pc);

                ImGui::TableNextColumn();
                ImGui::Text(
                    "%02X %02X %02X",
                    static_cast<unsigned int>(it->bytes[0]),
                    static_cast<unsigned int>(it->bytes[1]),
                    static_cast<unsigned int>(it->bytes[2])
                );

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(instruction.text.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%llu", static_cast<unsigned long long>(it->cycles));

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(flags.c_str());
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();
}

void Gui::DrawTms5501Debug()
{
    if (!_showTms5501Debug || !_debug)
    {
        return;
    }

    if (ImGui::Begin(
        "TMS5501 - Debug",
        &_showTms5501Debug,
        ImGuiWindowFlags_AlwaysAutoResize
    ))
    {
        const auto state = _debug->GetTms5501State();
        const std::string statusBits = ByteBits(state.status);
        const std::string maskBits = ByteBits(state.intMask);
        const std::string pendingBits = ByteBits(state.intStatus);
        const std::string outportBits = ByteBits(state.outport);

        ImGui::SeparatorText("Status");
        ImGui::Text("SSTATUS %02X  %s", static_cast<unsigned int>(state.status), statusBits.c_str());
        ImGui::TextDisabled("FE OVR RCV RBL XBE INT FBD SBD (bit 7 first)");

        ImGui::SeparatorText("Interrupts");
        ImGui::Text("Mask    %02X  %s", static_cast<unsigned int>(state.intMask), maskBits.c_str());
        ImGui::Text("Pending %02X  %s", static_cast<unsigned int>(state.intStatus), pendingBits.c_str());
        ImGui::Text("RST vector: %02X", static_cast<unsigned int>(state.pendingInterrupt));
        ImGui::TextDisabled("INT7..INT0: timer5, timer4, TX, RX, timer3, sensor, timer2, timer1");

        ImGui::SeparatorText("Timers");
        ImGui::Text(
            "1 %02X   2 %02X   3 %02X   4 %02X   5 %02X",
            static_cast<unsigned int>(state.timers[0]),
            static_cast<unsigned int>(state.timers[1]),
            static_cast<unsigned int>(state.timers[2]),
            static_cast<unsigned int>(state.timers[3]),
            static_cast<unsigned int>(state.timers[4])
        );

        ImGui::SeparatorText("Serial and I/O");
        ImGui::Text(
            "Rate %02X   DSC %02X   RX %02X   TX %02X   depth %u",
            static_cast<unsigned int>(state.rate),
            static_cast<unsigned int>(state.dscCmd),
            static_cast<unsigned int>(state.rxData),
            static_cast<unsigned int>(state.txData),
            static_cast<unsigned int>(state.txDataCount)
        );
        ImGui::Text(
            "Serial %s   Audio TXD %s",
            state.serialSelected ? "selected" : "idle",
            state.audioLevel ? "high" : "low"
        );
        ImGui::Text("Outport %02X  %s", static_cast<unsigned int>(state.outport), outportBits.c_str());
        ImGui::TextDisabled(
            "FDD1 %s   FDD2 %s",
            state.floppy1Selected ? "selected" : "idle",
            state.floppy2Selected ? "selected" : "idle"
        );
    }

    ImGui::End();
}

void Gui::DrawSmc5027Debug()
{
    if (!_showSmc5027Debug || !_debug)
    {
        return;
    }

    if (ImGui::Begin(
        "SMC5027 - Debug",
        &_showSmc5027Debug,
        ImGuiWindowFlags_AlwaysAutoResize
    ))
    {
        const auto state = _debug->GetSmc5027State();

        ImGui::SeparatorText("Registers");
        ImGui::Text(
            "Cursor X %u   Cursor Y %u",
            static_cast<unsigned int>(state.cursorX),
            static_cast<unsigned int>(state.cursorY)
        );
        ImGui::Text(
            "First display row %u of 32",
            static_cast<unsigned int>(state.firstDisplayRow)
        );

        ImGui::SeparatorText("Viewport");
        for (int row = 0; row < 32; row++)
        {
            if (row != 0 && row % 16 == 0)
            {
                ImGui::NewLine();
            }

            const bool visible = row == state.firstDisplayRow;
            ImGui::TextColored(
                visible
                    ? ImVec4(0.98f, 0.80f, 0.44f, 1.f)
                    : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled),
                "%02X ",
                row
            );
            ImGui::SameLine();
        }

        ImGui::NewLine();
        ImGui::TextDisabled("Highlighted row is the current scroll origin.");
    }

    ImGui::End();
}

void Gui::DrawReplayStatus()
{
    const bool active = _replayRecording || _replayProgress.active;

    if (!_showReplayStatus && !active)
    {
        return;
    }

    const auto formatTime = [](const uint64_t nanoseconds) {
        if (nanoseconds == std::numeric_limits<uint64_t>::max())
        {
            return std::string("none");
        }

        std::ostringstream out;
        out << (nanoseconds / 1000) << " us";
        return out.str();
    };

    if (ImGui::Begin("Replay Status", &_showReplayStatus, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Recording  %s", _replayRecording ? "active" : "off");
        ImGui::Text("Replay     %s", _replayProgress.active ? "active" : "idle");

        ImGui::Separator();

        ImGui::Text(
            "Now        %s",
            formatTime(_replayProgress.currentNanoseconds).c_str()
        );
        ImGui::Text(
            "Next input %s",
            formatTime(_replayProgress.nextEventNanoseconds).c_str()
        );
        ImGui::Text(
            "Stop       %s",
            _replayProgress.active
                ? formatTime(_replayProgress.stopNanoseconds).c_str()
                : "n/a"
        );
        ImGui::Text(
            "Input      %llu / %llu",
            static_cast<unsigned long long>(_replayProgress.appliedEvents),
            static_cast<unsigned long long>(_replayProgress.totalEvents)
        );

        ImGui::Separator();
        ImGui::TextDisabled("Times are in emulated machine time, not wall clock time.");
        ImGui::TextDisabled("Live keyboard input is ignored during replay.");
    }

    ImGui::End();
}

const char* Gui::MemoryRegion(const uint16_t address)
{
    if (address < 0x4000)
    {
        return "ROM";
    }

    if (address < 0x6000)
    {
        return "RAM";
    }

    if (address < 0x7000)
    {
        return "Video RAM alias";
    }

    if (address < 0x8000)
    {
        return "Video RAM";
    }

    return "RAM";
}

const char* Gui::PortDevice(const uint8_t port)
{
    if (port <= 0x1F)
    {
        return "TMS5501";
    }

    if (port >= 0x60 && port <= 0x7F)
    {
        return "SMC5027";
    }

    if (port >= 0x80 && port <= 0x9F)
    {
        return "CRT timing PROM";
    }

    return "unmapped";
}

std::string Gui::ByteBits(const uint8_t value)
{
    std::string bits(8, '0');

    for (std::size_t i = 0; i < bits.size(); i++)
    {
        bits[7 - i] = (value & (1U << i)) ? '1' : '0';
    }

    return bits;
}

bool Gui::ParseHexAddress(const char* text, uint16_t& address)
{
    if (!text || *text == '\0')
    {
        return false;
    }

    uint32_t value = 0;

    for (const char* cursor = text; *cursor != '\0'; ++cursor)
    {
        uint32_t digit = 0;

        if (*cursor >= '0' && *cursor <= '9')
        {
            digit = static_cast<uint32_t>(*cursor - '0');
        }
        else if (*cursor >= 'a' && *cursor <= 'f')
        {
            digit = static_cast<uint32_t>(*cursor - 'a') + 10;
        }
        else if (*cursor >= 'A' && *cursor <= 'F')
        {
            digit = static_cast<uint32_t>(*cursor - 'A') + 10;
        }
        else
        {
            return false;
        }

        value = value * 16 + digit;

        if (value > UINT16_MAX)
        {
            return false;
        }
    }

    address = static_cast<uint16_t>(value);
    return true;
}

void Gui::OpenBrowser(const BrowserMode mode, const int drive)
{
    _browserMode = mode;
    _browserDrive = drive;

    std::error_code error;
    fs::path start = _settings->GetString("last_disk_dir");

    if (start.empty() || !fs::is_directory(start, error))
    {
        start = fs::is_directory("disks", error) ? fs::path("disks") : fs::current_path(error);
    }

    _browserDir = fs::absolute(start, error);

    std::memset(_fileName, 0, sizeof(_fileName));

    if (mode == BrowserMode::SaveAs)
    {
        const auto floppy = _emulator.GetFloppy(drive);
        const std::string name = floppy ? fs::path(floppy->GetMountedPath()).filename().string() : std::string();
        std::strncpy(_fileName, name.c_str(), sizeof(_fileName) - 1);
    }
    else if (mode == BrowserMode::SaveReplay)
    {
        std::strncpy(_fileName, "replay.cc2replay.json", sizeof(_fileName) - 1);
    }

    _openBrowser = true;
}

void Gui::DrawBrowser()
{
    bool open = true;

    ImGui::SetNextWindowSize(ImVec2(420.f * _uiScale, 320.f * _uiScale), ImGuiCond_Appearing);

    if (!ImGui::BeginPopupModal(BrowserPopup, &open))
    {
        return;
    }

    const bool saving = _browserMode != BrowserMode::Mount;
    const char* extension = _browserMode == BrowserMode::SaveReplay
        ? ReplayExtension
        : DiskExtension;

    if (_browserMode == BrowserMode::SaveReplay)
    {
        ImGui::TextUnformatted("Save replay bundle to");
    }
    else
    {
        ImGui::Text(
            "%s Drive %s",
            saving ? "Save disk from" : "Mount into",
            DriveLabel(_browserDrive)
        );
    }

    ImGui::TextWrapped("%s", _browserDir.string().c_str());

    if (ImGui::Button("Up") && _browserDir.has_parent_path())
    {
        _browserDir = _browserDir.parent_path();
    }

    std::vector<fs::path> directories;
    std::vector<fs::path> files;

    std::error_code error;
    for (fs::directory_iterator it(_browserDir, fs::directory_options::skip_permission_denied, error), end;
         !error && it != end;
         it.increment(error))
    {
        std::error_code entryError;

        if (it->is_directory(entryError))
        {
            directories.push_back(it->path());
        }
        else if (it->path().extension() == extension)
        {
            files.push_back(it->path());
        }
    }

    std::sort(directories.begin(), directories.end());
    std::sort(files.begin(), files.end());

    bool accepted = false;

    ImGui::BeginChild("entries", ImVec2(0.f, -ImGui::GetFrameHeightWithSpacing() * 2.f), ImGuiChildFlags_Borders);

    fs::path enter;

    for (const fs::path& directory : directories)
    {
        const std::string label = "[dir] " + directory.filename().string();

        if (ImGui::Selectable(label.c_str()))
        {
            enter = directory;
        }
    }

    for (const fs::path& file : files)
    {
        const std::string name = file.filename().string();

        if (ImGui::Selectable(name.c_str(), name == _fileName, ImGuiSelectableFlags_AllowDoubleClick))
        {
            std::memset(_fileName, 0, sizeof(_fileName));
            std::strncpy(_fileName, name.c_str(), sizeof(_fileName) - 1);

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                accepted = true;
            }
        }
    }

    ImGui::EndChild();

    if (!enter.empty())
    {
        _browserDir = enter;
    }

    ImGui::InputText("File name", _fileName, sizeof(_fileName), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::TextDisabled("Double-click a file or press Enter to continue.");

    ImGui::BeginDisabled(_fileName[0] == '\0');
    if (ImGui::Button(saving ? "Save" : "Mount"))
    {
        accepted = true;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        open = false;
    }

    if (accepted && _fileName[0] != '\0')
    {
        AcceptBrowser();
        open = false;
    }

    if (!open)
    {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Gui::AcceptBrowser()
{
    fs::path target = _browserDir / _fileName;

    if (_browserMode == BrowserMode::SaveAs && target.extension().empty())
    {
        target += DiskExtension;
    }

    if (_browserMode == BrowserMode::SaveReplay)
    {
        if (target.extension().empty())
        {
            target += ReplayFileNameSuffix;
        }

        _settings->SetString("last_disk_dir", _browserDir.string());

        if (_startReplayRecording)
        {
            _startReplayRecording(target.string());
        }

        return;
    }

    if (_browserMode == BrowserMode::SaveAs)
    {
        const auto floppy = _emulator.GetFloppy(_browserDrive);
        std::string saveError;

        if (floppy && floppy->SaveAs(target.string(), saveError))
        {
            _settings->SetString(DiskSettingKey(_browserDrive), target.string());
            _settings->SetString("last_disk_dir", _browserDir.string());
            ShowStatus("Saved disk " + target.filename().string());
        }
        else
        {
            _errorMessage = saveError;
        }

        return;
    }

    if (MountDisk(_browserDrive, target.string()))
    {
        ShowStatus("Mounted " + target.filename().string() + " in drive " + DriveLabel(_browserDrive));
    }
}

void Gui::GuardUnsaved(const std::vector<int>& drives, std::function<void()> action)
{
    std::vector<int> modified;

    for (const int drive : drives)
    {
        if (IsModified(drive))
        {
            modified.push_back(drive);
        }
    }

    if (modified.empty())
    {
        action();
        return;
    }

    _pendingDrives = std::move(modified);
    _pendingAction = std::move(action);
    _openUnsaved = true;
}

bool Gui::IsModified(const int drive) const
{
    const auto floppy = _emulator.GetFloppy(drive);

    return floppy && floppy->IsMounted() && floppy->IsModified();
}

void Gui::DrawUnsavedPrompt()
{
    if (!ImGui::BeginPopupModal(UnsavedPopup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }

    for (const int drive : _pendingDrives)
    {
        ImGui::Text("Disk %s has unsaved changes.", DriveLabel(drive));
    }

    std::function<void()> run;

    if (ImGui::Button("Save and continue"))
    {
        bool allSaved = true;

        for (const int drive : _pendingDrives)
        {
            const auto floppy = _emulator.GetFloppy(drive);
            std::string saveError;

            if (floppy && !floppy->Save(saveError))
            {
                _errorMessage = saveError;
                allSaved = false;
            }
        }

        if (allSaved)
        {
            run = std::move(_pendingAction);
        }

        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("Discard"))
    {
        run = std::move(_pendingAction);
        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();

    // Run after the popup so actions that open other popups do not nest inside this one.
    if (run)
    {
        _pendingAction = nullptr;
        _pendingDrives.clear();
        run();
    }
}

void Gui::DrawError()
{
    if (!ImGui::BeginPopupModal(ErrorPopup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }

    ImGui::TextWrapped("%s", _errorMessage.c_str());

    if (ImGui::Button("OK"))
    {
        _errorMessage.clear();
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

bool Gui::MountDisk(const int drive, const std::string& path)
{
    const auto floppy = _emulator.GetFloppy(drive);

    if (!floppy)
    {
        return false;
    }

    std::string error;

    if (!floppy->Mount(path, error))
    {
        _errorMessage = error;
        return false;
    }

    _settings->SetString(DiskSettingKey(drive), path);
    _settings->SetString("last_disk_dir", fs::path(path).parent_path().string());

    return true;
}

void Gui::EjectDisk(const int drive)
{
    if (const auto floppy = _emulator.GetFloppy(drive))
    {
        floppy->Eject();
        _settings->SetString(DiskSettingKey(drive), "");
        ShowStatus(std::string("Ejected drive ") + DriveLabel(drive));
    }
}

void Gui::SaveDisk(const int drive)
{
    const auto floppy = _emulator.GetFloppy(drive);
    std::string error;

    if (floppy && !floppy->Save(error))
    {
        _errorMessage = error;
        return;
    }

    if (floppy)
    {
        ShowStatus("Saved " + fs::path(floppy->GetMountedPath()).filename().string());
    }
}

void Gui::SetFullscreen(const bool fullscreen)
{
    _settings->SetBool("fullscreen", fullscreen);
    _windowChanged = true;
}

void Gui::SetScale(const int scale)
{
    _settings->SetInt("window_scale", scale);
    _settings->SetBool("fullscreen", false);
    _windowChanged = true;
}

void Gui::SetSmooth(const bool smooth)
{
    _settings->SetBool("smooth_scaling", smooth);
    _display->SetSmooth(smooth);
}

const char* Gui::DriveLabel(const int drive)
{
    return drive == 0 ? "A" : "B";
}

const char* Gui::DiskSettingKey(const int drive)
{
    return drive == 0 ? "disk_a_path" : "disk_b_path";
}

void Gui::HelpMarker(const char* text)
{
    ImGui::TextDisabled("(?)");

    if (!ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
    {
        return;
    }

    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
}
