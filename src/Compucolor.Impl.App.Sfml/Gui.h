#pragma once

#include <filesystem>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include <Compucolor/CompucolorEmulatorProcessor.h>

#include "Display.h"
#include "Settings.h"

namespace Compucolor::Impl::App::Sfml
{
    // Menu bar, disk panel and settings windows drawn over the emulator screen with Dear ImGui.
    class Gui {
        public:
            Gui(
                std::shared_ptr<sf::RenderWindow> window,
                std::shared_ptr<Settings> settings,
                Compucolor::CompucolorEmulatorProcessor& emulator,
                std::shared_ptr<Display> display
            );

            // Must be called again after the window is recreated.
            bool Init();
            void Shutdown();

            void SetQuitHandler(std::function<void()> handler);

            // Applies saved machine settings and mounts the saved disks.
            void ApplyStartupSettings();

            void ProcessEvent(const sf::Event& event);
            void BeginFrame(sf::Time elapsed);
            void Render();

            [[nodiscard]] bool WantsKeyboard() const;

            // Closes immediately unless a disk has unsaved changes.
            void RequestQuit();

            void ToggleMenu();

            void SaveScreenshot();

            // True once if the window mode or scale changed and the window must be recreated.
            bool ConsumeWindowChange();

        private:
            enum class BrowserMode { Mount, SaveAs };

            std::shared_ptr<sf::RenderWindow> _window;
            std::shared_ptr<Settings> _settings;
            Compucolor::CompucolorEmulatorProcessor& _emulator;
            std::shared_ptr<Display> _display;
            std::function<void()> _quit;

            bool _ready = false;

            // The window must not reference ImGui's cursors when they are freed.
            std::optional<sf::Cursor> _arrowCursor;
            bool _menuVisible = true;
            bool _showDiskPanel = true;
            bool _showSettings = false;
            bool _showKeys = false;
            bool _showAbout = false;
            bool _windowChanged = false;
            float _uiScale = 0.f;

            bool _openBrowser = false;
            bool _openUnsaved = false;
            BrowserMode _browserMode = BrowserMode::Mount;
            int _browserDrive = 0;
            std::filesystem::path _browserDir;
            char _fileName[256] = {};

            std::function<void()> _pendingAction;
            std::vector<int> _pendingDrives;
            std::string _errorMessage;

            std::string _status;
            std::chrono::steady_clock::time_point _statusUntil;

            void ShowStatus(const std::string& message);
            void DrawStatus();

            void DrawMenu();
            void DrawDiskPanel();
            void DrawSettings();
            void DrawKeyHelp();
            void DrawAbout();
            void DrawBrowser();
            void DrawUnsavedPrompt();
            void DrawError();

            void DrawDriveControls(int drive);

            // Keeps the UI readable as the window grows by scaling fonts and widget sizes.
            void UpdateScale();

            void ApplyMachineSettings();
            bool MountDisk(int drive, const std::string& path);
            void EjectDisk(int drive);
            void SaveDisk(int drive);
            void OpenBrowser(BrowserMode mode, int drive);

            // Runs the action now, or after the user resolves unsaved changes on the given drives.
            void GuardUnsaved(const std::vector<int>& drives, std::function<void()> action);
            [[nodiscard]] bool IsModified(int drive) const;

            void SetFullscreen(bool fullscreen);
            void SetScale(int scale);
            void SetSmooth(bool smooth);

            static const char* DriveLabel(int drive);
            static const char* DiskSettingKey(int drive);
    };
}
