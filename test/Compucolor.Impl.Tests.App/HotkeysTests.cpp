#include <set>
#include <string>

#include <gtest/gtest.h>

#include "Hotkeys.h"

using Compucolor::Impl::App::Sfml::HotkeyAction;
using Compucolor::Impl::App::Sfml::HotkeyDefinitions;
using Compucolor::Impl::App::Sfml::IsAltKey;
using Compucolor::Impl::App::Sfml::MatchHotkey;
using Compucolor::Impl::App::Sfml::ScaleHotkey;
using Compucolor::Impl::App::Sfml::Shortcut;

namespace
{
    std::string Chord(const sf::Keyboard::Key key, const bool shift)
    {
        return (shift ? "Shift+" : "") + std::to_string(static_cast<int>(key));
    }
}

TEST(Hotkeys, ChordsAreUnique) {
    //Arrange
    std::set<std::string> chords;

    //Act / Assert
    for (const auto& definition : HotkeyDefinitions)
    {
        const std::string chord = Chord(definition.key, definition.shift);
        EXPECT_TRUE(chords.insert(chord).second) << "Duplicate chord: " << chord;
    }
}

TEST(Hotkeys, EveryActionHasShortcutAndDescription) {
    //Act / Assert
    for (const auto& definition : HotkeyDefinitions)
    {
        EXPECT_NE(std::string(Shortcut(definition.action)), "") << static_cast<int>(definition.action);
        EXPECT_NE(std::string(definition.description), "") << Shortcut(definition.action);
    }
}

TEST(Hotkeys, MatchRequiresExactShiftState) {
    //Act / Assert
    EXPECT_EQ(MatchHotkey(sf::Keyboard::Key::S, false), HotkeyAction::SmoothScaling);
    EXPECT_EQ(MatchHotkey(sf::Keyboard::Key::S, true), HotkeyAction::SaveState);
    EXPECT_EQ(MatchHotkey(sf::Keyboard::Key::Q, false), std::nullopt);
    EXPECT_EQ(MatchHotkey(sf::Keyboard::Key::Q, true), HotkeyAction::Quit);
}

TEST(Hotkeys, AltKeysAreRecognizedAsTheHostModifier) {
    //Act / Assert
    EXPECT_TRUE(IsAltKey(sf::Keyboard::Key::LAlt));
    EXPECT_TRUE(IsAltKey(sf::Keyboard::Key::RAlt));
    EXPECT_FALSE(IsAltKey(sf::Keyboard::Key::LControl));
}

TEST(Hotkeys, ScaleActionsAreContiguous) {
    //Act / Assert
    EXPECT_EQ(ScaleHotkey(1), HotkeyAction::Scale1);
    EXPECT_EQ(ScaleHotkey(4), HotkeyAction::Scale4);
    EXPECT_EQ(std::string(Shortcut(ScaleHotkey(3))), "Alt+3");
}
