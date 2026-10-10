#pragma once

#include <optional>

#include <SFML/Graphics.hpp>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

namespace Compucolor::Impl::App::Sfml
{
    class KeyboardTranslator {
        public:
            KeyboardTranslator();

            static std::optional<Compucolor::Keyboard::CompucolorIIKey> ConvertKey(sf::Keyboard::Key key);

            // US layout character for digits and punctuation keys; letters and other keys return nothing.
            static std::optional<char> ToAscii(sf::Keyboard::Key key, bool shift);
    };
}