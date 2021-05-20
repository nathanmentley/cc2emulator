#include "KeyboardTranslator.h"

Compucolor::App::Impl::Smfl::KeyboardTranslator::KeyboardTranslator()
{
}

Compucolor::Keyboard::CompucolorIIKey Compucolor::App::Impl::Smfl::KeyboardTranslator::ConvertKey(sf::Keyboard::Key key)
{
    return Compucolor::Keyboard::CompucolorIIKey::C;
}