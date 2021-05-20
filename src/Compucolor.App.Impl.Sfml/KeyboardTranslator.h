#pragma once

#include <SFML/Graphics.hpp>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

namespace Compucolor::App::Impl::Sfml
{
    class KeyboardTranslator {
        public:
            KeyboardTranslator();

            Keyboard::CompucolorIIKey ConvertKey(sf::Keyboard::Key key);
    };
}