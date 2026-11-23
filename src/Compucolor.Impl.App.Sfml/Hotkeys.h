#pragma once

#include <array>
#include <cstddef>
#include <optional>

#include <SFML/Window/Keyboard.hpp>

namespace Compucolor::Impl::App::Sfml
{
    // Host-only shortcuts. Alt is never forwarded to the emulated keyboard, so
    // every Alt chord is safe to use for the UI without disturbing Compucolor
    // software. Scale actions are kept contiguous for ScaleHotkey().
    enum class HotkeyAction {
        Fullscreen,
        Scale1,
        Scale2,
        Scale3,
        Scale4,
        SmoothScaling,
        CrtEffects,
        Sound,
        ToggleMenu,
        Screenshot,
        SaveState,
        LoadState,
        ToggleReplayRecording,
        Pause,
        RealSpeed,
        DiskPanel,
        OnScreenKeyboard,
        TraceRecording,
        ClearTraces,
        CpuDebug,
        Preferences,
        KeyboardHelp,
        About,
        Quit
    };

    struct HotkeyDefinition {
        HotkeyAction action;
        sf::Keyboard::Key key;
        bool shift;
        const char* shortcut;
        const char* description;
    };

    inline constexpr auto HotkeyDefinitions = std::to_array<HotkeyDefinition>({
        {HotkeyAction::Fullscreen,           sf::Keyboard::Key::Enter, false, "Alt+Enter",    "Toggle fullscreen"},
        {HotkeyAction::Scale1,               sf::Keyboard::Key::Num1,  false, "Alt+1",        "Window scale 1x"},
        {HotkeyAction::Scale2,               sf::Keyboard::Key::Num2,  false, "Alt+2",        "Window scale 2x"},
        {HotkeyAction::Scale3,               sf::Keyboard::Key::Num3,  false, "Alt+3",        "Window scale 3x"},
        {HotkeyAction::Scale4,               sf::Keyboard::Key::Num4,  false, "Alt+4",        "Window scale 4x"},
        {HotkeyAction::SmoothScaling,        sf::Keyboard::Key::S,     false, "Alt+S",        "Toggle smooth scaling"},
        {HotkeyAction::CrtEffects,           sf::Keyboard::Key::F,     true,  "Alt+Shift+F",  "Toggle CRT effects"},
        {HotkeyAction::Sound,                sf::Keyboard::Key::A,     false, "Alt+A",        "Toggle sound"},
        {HotkeyAction::ToggleMenu,           sf::Keyboard::Key::M,     false, "Alt+M",        "Show or hide the menu"},
        {HotkeyAction::Screenshot,           sf::Keyboard::Key::P,     false, "Alt+P",        "Save a screenshot"},
        {HotkeyAction::SaveState,            sf::Keyboard::Key::S,     true,  "Alt+Shift+S",  "Save emulator state"},
        {HotkeyAction::LoadState,            sf::Keyboard::Key::L,     true,  "Alt+Shift+L",  "Load emulator state"},
        {HotkeyAction::ToggleReplayRecording,sf::Keyboard::Key::R,     false, "Alt+R",        "Start or stop replay recording"},
        {HotkeyAction::Pause,                sf::Keyboard::Key::P,     true,  "Alt+Shift+P",  "Pause or resume"},
        {HotkeyAction::RealSpeed,            sf::Keyboard::Key::R,     true,  "Alt+Shift+R",  "Toggle real speed"},
        {HotkeyAction::DiskPanel,            sf::Keyboard::Key::D,     false, "Alt+D",        "Toggle the disk panel"},
        {HotkeyAction::OnScreenKeyboard,     sf::Keyboard::Key::K,     false, "Alt+K",        "Show or hide the on-screen keyboard"},
        {HotkeyAction::TraceRecording,       sf::Keyboard::Key::T,     false, "Alt+T",        "Toggle trace recording"},
        {HotkeyAction::ClearTraces,          sf::Keyboard::Key::T,     true,  "Alt+Shift+T",  "Clear traces"},
        {HotkeyAction::CpuDebug,             sf::Keyboard::Key::C,     false, "Alt+C",        "Toggle the CPU debug window"},
        {HotkeyAction::Preferences,          sf::Keyboard::Key::O,     false, "Alt+O",        "Open preferences"},
        {HotkeyAction::KeyboardHelp,         sf::Keyboard::Key::H,     false, "Alt+H",        "Show keyboard help"},
        {HotkeyAction::About,                sf::Keyboard::Key::I,     false, "Alt+I",        "Show about"},
        {HotkeyAction::Quit,                 sf::Keyboard::Key::Q,     true,  "Alt+Shift+Q",  "Quit the emulator"}
    });

    [[nodiscard]] constexpr const char* Shortcut(const HotkeyAction action) noexcept
    {
        for (const HotkeyDefinition& definition : HotkeyDefinitions)
        {
            if (definition.action == action)
            {
                return definition.shortcut;
            }
        }

        return "";
    }

    [[nodiscard]] constexpr const char* HotkeyDescription(const HotkeyAction action) noexcept
    {
        for (const HotkeyDefinition& definition : HotkeyDefinitions)
        {
            if (definition.action == action)
            {
                return definition.description;
            }
        }

        return "";
    }

    [[nodiscard]] constexpr std::optional<HotkeyAction> MatchHotkey(
        const sf::Keyboard::Key key,
        const bool shift
    ) noexcept
    {
        for (const HotkeyDefinition& definition : HotkeyDefinitions)
        {
            if (definition.key == key && definition.shift == shift)
            {
                return definition.action;
            }
        }

        return std::nullopt;
    }

    [[nodiscard]] constexpr bool IsAltKey(const sf::Keyboard::Key key) noexcept
    {
        return key == sf::Keyboard::Key::LAlt || key == sf::Keyboard::Key::RAlt;
    }

    [[nodiscard]] constexpr HotkeyAction ScaleHotkey(const int option) noexcept
    {
        return static_cast<HotkeyAction>(
            static_cast<int>(HotkeyAction::Scale1) + (option - 1)
        );
    }
}
