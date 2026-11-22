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

#include "AudioOutput.h"
#include <Compucolor/App/DebugFilter.h>
#include <Compucolor/App/DebugHub.h>
#include <Compucolor/App/DebugMonitor.h>
#include "Display.h"
#include <Compucolor/App/Replay/ReplayInputSink.h>
#include "Settings.h"

namespace Compucolor::Impl::App::Sfml
{
    // Menu bar, disk panel, settings and debug windows drawn over the emulator
    // screen with Dear ImGui.
    class Gui {
        public:
            Gui(
                std::shared_ptr<sf::RenderWindow> window,
                std::shared_ptr<Settings> settings,
                Compucolor::CompucolorEmulatorProcessor& emulator,
                std::shared_ptr<DebugMonitor> debug,
                std::shared_ptr<DebugHub> debugHub,
                std::shared_ptr<Display> display,
                std::shared_ptr<AudioOutput> audio
            );

            // Must be called again after the window is recreated.
            bool Init();
            void Shutdown();

            void SetQuitHandler(std::function<void()> handler);

            // Hooks used by the File menu replay actions. The path is chosen
            // before recording starts; stopping writes the bundle immediately.
            void SetReplayHandlers(
                std::function<void(const std::string&)> startRecording,
                std::function<void()> stopRecording
            );

            // Updates replay indicators and the progress window from host-safe
            // replay state published by Program.
            void SetReplayState(
                bool recording,
                const Compucolor::Impl::App::Replay::ReplayProgress& progress
            );

            void ReportError(const std::string& message);
            void ShowMessage(const std::string& message);

            // Applies saved machine settings and mounts the saved disks.
            void ApplyStartupSettings();

            // Forwards SFML events to ImGui's menubar, dialogs and file browser.
            void ProcessEvent(const sf::Event& event);

            // Advances animations/status messages and lays out the next frame.
            void BeginFrame(sf::Time elapsed);

            // Draws the overlay inside the Display::Render callback.
            void Render();

            // True when a text field or popup should receive keyboard input.
            [[nodiscard]] bool WantsKeyboard() const;

            // Closes immediately unless a disk has unsaved changes.
            void RequestQuit();

            void ToggleMenu();

            // Saves the raw emulated frame to the screenshots directory.
            void SaveScreenshot();

            // Sound controls used by the Sound menu, Preferences and the keyboard shortcut.
            void SetSoundEnabled(bool enabled);
            void SetSoundVolume(int volume);
            void ToggleSound();

            // True once if the window mode or scale changed and the window must be recreated.
            bool ConsumeWindowChange();

        private:
            enum class BrowserMode { Mount, SaveAs, SaveReplay };

            std::shared_ptr<sf::RenderWindow> _window;
            std::shared_ptr<Settings> _settings;
            Compucolor::CompucolorEmulatorProcessor& _emulator;
            std::shared_ptr<DebugMonitor> _debug;
            std::shared_ptr<DebugHub> _debugHub;
            std::shared_ptr<Display> _display;
            std::shared_ptr<AudioOutput> _audio;
            std::function<void()> _quit;
            std::function<void(const std::string&)> _startReplayRecording;
            std::function<void()> _stopReplayRecording;

            bool _ready = false;

            // The window must not reference ImGui's cursors when they are freed.
            std::optional<sf::Cursor> _arrowCursor;
            bool _menuVisible = true;
            bool _showDiskPanel = false;
            bool _showSettings = false;
            bool _showKeys = false;
            bool _showAbout = false;
            bool _windowChanged = false;
            float _uiScale = 0.f;

            bool _showCpuDebug = false;
            bool _showMemoryDebug = false;
            bool _showPortDebug = false;
            bool _showTraceDebug = false;
            bool _showTms5501Debug = false;
            bool _showSmc5027Debug = false;
            bool _showReplayStatus = false;
            bool _replayRecording = false;
            Compucolor::Impl::App::Replay::ReplayProgress _replayProgress{};
            bool _debugRecording = false;

            DebugFilter _instructionFilter{};
            DebugFilter _portFilter{};
            char _instructionFilterText[64] = {};

            uint16_t _memoryViewAddress = 0x4000;
            char _memoryAddressText[5] = "4000";

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
            void DrawMenuStatus();

            void DrawMenu();
            void DrawDiskPanel();
            void DrawSettings();
            void DrawKeyHelp();
            void DrawAbout();
            void DrawDebugWindows();
            void DrawCpuDebug();
            void DrawMemoryDebug();
            void DrawPortDebug();
            void DrawTraceDebug();
            void DrawTms5501Debug();
            void DrawSmc5027Debug();
            void DrawReplayStatus();
            void DrawBrowser();
            void DrawUnsavedPrompt();
            void DrawError();

            void DrawDriveControls(int drive);

            // Keeps the UI readable as the window grows by scaling fonts and widget sizes.
            void UpdateScale();

            void ApplyMachineSettings();
            void ApplySoundSettings();

            // Delegates to the processor's compucolor-state.json and shows errors as a modal.
            void SaveState();
            void LoadState();
            void AcceptBrowser();
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
            static const char* MemoryRegion(uint16_t address);
            static const char* PortDevice(uint8_t port);
            static std::string ByteBits(uint8_t value);
            static bool ParseHexAddress(const char* text, uint16_t& address);
            static void HelpMarker(const char* text);
    };
}
