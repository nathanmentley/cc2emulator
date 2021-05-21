#pragma once

#include <optional>

#include <SFML/Graphics.hpp>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

namespace Compucolor::Impl::App::Sfml
{
    class KeyboardTranslator {
        public:
            KeyboardTranslator();

            std::optional<Compucolor::Keyboard::CompucolorIIKey> ConvertKey(sf::Keyboard::Key key);
    };
}