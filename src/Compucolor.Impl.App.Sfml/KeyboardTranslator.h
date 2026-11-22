#pragma once

#include <optional>

#include <SFML/Graphics.hpp>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

namespace Compucolor::Impl::App::Sfml
{
    // Translates SFML key events into Compucolor II matrix keys. Special keys
    // map directly; digits and punctuation are resolved from the character the
    // PC key produces so that shifted symbols match the original keyboard.
    class KeyboardTranslator {
        public:
            KeyboardTranslator();

            // Direct key mapping; returns nullopt for keys handled only as characters.
            static std::optional<Compucolor::Keyboard::CompucolorIIKey> ConvertKey(sf::Keyboard::Key key);

            // US-layout character for a key: unshifted letters become uppercase,
            // shifted letters lowercase (the CC2 is a typewriter keyboard).
            // Digits and punctuation follow the US shifted symbols; other keys return nullopt.
            static std::optional<char> ToAscii(sf::Keyboard::Key key, bool shift);
    };
}