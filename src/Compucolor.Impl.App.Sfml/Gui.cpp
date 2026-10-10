#include "Gui.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
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
}

Gui::Gui(
    std::shared_ptr<sf::RenderWindow> window,
    std::shared_ptr<Settings> settings,
    Compucolor::CompucolorEmulatorProcessor& emulator,
    std::shared_ptr<Display> display
):
    _window(std::move(window)),
    _settings(std::move(settings)),
    _emulator(emulator),
    _display(std::move(display))
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

void Gui::ApplyStartupSettings()
{
    ApplyMachineSettings();

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
    const float scale = std::clamp(std::round(height / 360.f * 4.f) / 4.f, 1.f, 4.f);

    if (scale == _uiScale)
    {
        return;
    }

    _uiScale = scale;

    ImGuiStyle style;
    ImGui::StyleColorsDark(&style);
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
    ImGui::SetNextWindowBgAlpha(0.8f);

    if (ImGui::Begin("##status", nullptr, flags))
    {
        ImGui::TextUnformatted(_status.c_str());
    }

    ImGui::End();
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
        for (int drive = 0; drive < 2; drive++)
        {
            ImGui::PushID(drive);

            const std::string mountLabel = std::string("Mount Disk ") + DriveLabel(drive) + "...";
            if (ImGui::MenuItem(mountLabel.c_str()))
            {
                GuardUnsaved({drive}, [this, drive] { OpenBrowser(BrowserMode::Mount, drive); });
            }

            ImGui::PopID();
        }

        ImGui::Separator();

        for (int drive = 0; drive < 2; drive++)
        {
            ImGui::PushID(drive);
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
        if (ImGui::SliderInt("Window scale", &scale, 1, 8))
        {
            SetScale(scale);
        }

        bool smooth = _settings->GetBool("smooth_scaling");
        if (ImGui::Checkbox("Smooth scaling", &smooth))
        {
            SetSmooth(smooth);
        }

        ImGui::SeparatorText("Machine");

        int charset = _settings->GetString("charset") == "lowercase" ? 1 : 0;
        const char* charsets[] = {"Standard", "Lowercase"};
        if (ImGui::Combo("Character set", &charset, charsets, 2))
        {
            _settings->SetString("charset", charset == 1 ? "lowercase" : "standard");
            _emulator.SetLowercaseCharset(charset == 1);
        }

        int rom = _settings->GetString("rom_version") == "v6.78" ? 0 : 1;
        const char* roms[] = {"v6.78", "v8.79"};
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
        ImGui::BulletText("Alt+M: show or hide the menu");
        ImGui::BulletText("Alt+P: save a screenshot");

        ImGui::SeparatorText("PC keys on the Compucolor");
        ImGui::BulletText("Alt / Windows key: REPEAT");
        ImGui::BulletText("Ctrl: CONTROL");
        ImGui::BulletText("Symbols follow the printed character, e.g. Shift+; is ':' and Shift+7 is '&'");
        ImGui::BulletText("Arrow keys, Home, Tab: cursor keys");
        ImGui::BulletText("Insert / Delete: insert and delete character");
        ImGui::BulletText("Page Up / Page Down: erase page / erase line");
        ImGui::BulletText("End / Pause: BREAK");
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
        ImGui::TextDisabled("A Compucolor II Emulator.");
    }

    ImGui::End();
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

    const bool saving = _browserMode == BrowserMode::SaveAs;

    ImGui::Text("%s Drive %s", saving ? "Save disk from" : "Mount into", DriveLabel(_browserDrive));
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
        else if (it->path().extension() == DiskExtension)
        {
            files.push_back(it->path());
        }
    }

    std::sort(directories.begin(), directories.end());
    std::sort(files.begin(), files.end());

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
        }
    }

    ImGui::EndChild();

    if (!enter.empty())
    {
        _browserDir = enter;
    }

    ImGui::InputText("File name", _fileName, sizeof(_fileName));

    bool accepted = false;

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

    if (accepted)
    {
        fs::path target = _browserDir / _fileName;

        if (saving && target.extension().empty())
        {
            target += DiskExtension;
        }

        if (saving)
        {
            const auto floppy = _emulator.GetFloppy(_browserDrive);
            std::string saveError;

            if (floppy && floppy->SaveAs(target.string(), saveError))
            {
                _settings->SetString(DiskSettingKey(_browserDrive), target.string());
                _settings->SetString("last_disk_dir", _browserDir.string());
            }
            else
            {
                _errorMessage = saveError;
            }
        }
        else
        {
            MountDisk(_browserDrive, target.string());
        }

        open = false;
    }

    if (!open)
    {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
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
    }
}

void Gui::SaveDisk(const int drive)
{
    const auto floppy = _emulator.GetFloppy(drive);
    std::string error;

    if (floppy && !floppy->Save(error))
    {
        _errorMessage = error;
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
