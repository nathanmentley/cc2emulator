#include "KeyboardTranslator.h"

Compucolor::App::Impl::Sfml::KeyboardTranslator::KeyboardTranslator()
{
}

std::optional<Compucolor::Keyboard::CompucolorIIKey> Compucolor::App::Impl::Sfml::KeyboardTranslator::ConvertKey(sf::Keyboard::Key key)
{
    return {};//Compucolor::Keyboard::CompucolorIIKey::C;
}