#include "KeyboardTranslator.h"

Compucolor::Impl::App::Sfml::KeyboardTranslator::KeyboardTranslator() = default;

std::optional<Compucolor::Keyboard::CompucolorIIKey> Compucolor::Impl::App::Sfml::KeyboardTranslator::ConvertKey(
    const sf::Keyboard::Key key
)
{
    switch (key)
    {
        case sf::Keyboard::Key::A:
            return Compucolor::Keyboard::CompucolorIIKey::A;
        case sf::Keyboard::Key::B:
            return Compucolor::Keyboard::CompucolorIIKey::B;
        case sf::Keyboard::Key::C:
            return Compucolor::Keyboard::CompucolorIIKey::C;
        case sf::Keyboard::Key::D:
            return Compucolor::Keyboard::CompucolorIIKey::D;
        case sf::Keyboard::Key::E:
            return Compucolor::Keyboard::CompucolorIIKey::E;
        case sf::Keyboard::Key::F:
            return Compucolor::Keyboard::CompucolorIIKey::F;
        case sf::Keyboard::Key::G:
            return Compucolor::Keyboard::CompucolorIIKey::G;
        case sf::Keyboard::Key::H:
            return Compucolor::Keyboard::CompucolorIIKey::H;
        case sf::Keyboard::Key::I:
            return Compucolor::Keyboard::CompucolorIIKey::I;
        case sf::Keyboard::Key::J:
            return Compucolor::Keyboard::CompucolorIIKey::J;
        case sf::Keyboard::Key::K:
            return Compucolor::Keyboard::CompucolorIIKey::K;
        case sf::Keyboard::Key::L:
            return Compucolor::Keyboard::CompucolorIIKey::L;
        case sf::Keyboard::Key::M:
            return Compucolor::Keyboard::CompucolorIIKey::M;
        case sf::Keyboard::Key::N:
            return Compucolor::Keyboard::CompucolorIIKey::N;
        case sf::Keyboard::Key::O:
            return Compucolor::Keyboard::CompucolorIIKey::O;
        case sf::Keyboard::Key::P:
            return Compucolor::Keyboard::CompucolorIIKey::P;
        case sf::Keyboard::Key::Q:
            return Compucolor::Keyboard::CompucolorIIKey::Q;
        case sf::Keyboard::Key::R:
            return Compucolor::Keyboard::CompucolorIIKey::R;
        case sf::Keyboard::Key::S:
            return Compucolor::Keyboard::CompucolorIIKey::S;
        case sf::Keyboard::Key::T:
            return Compucolor::Keyboard::CompucolorIIKey::T;
        case sf::Keyboard::Key::U:
            return Compucolor::Keyboard::CompucolorIIKey::U;
        case sf::Keyboard::Key::V:
            return Compucolor::Keyboard::CompucolorIIKey::V;
        case sf::Keyboard::Key::W:
            return Compucolor::Keyboard::CompucolorIIKey::W;
        case sf::Keyboard::Key::X:
            return Compucolor::Keyboard::CompucolorIIKey::X;
        case sf::Keyboard::Key::Y:
            return Compucolor::Keyboard::CompucolorIIKey::Y;
        case sf::Keyboard::Key::Z:
            return Compucolor::Keyboard::CompucolorIIKey::Z;
        case sf::Keyboard::Key::Num0:
            return Compucolor::Keyboard::CompucolorIIKey::Zero;
        case sf::Keyboard::Key::Num1:
            return Compucolor::Keyboard::CompucolorIIKey::One;
        case sf::Keyboard::Key::Num2:
            return Compucolor::Keyboard::CompucolorIIKey::Two;
        case sf::Keyboard::Key::Num3:
            return Compucolor::Keyboard::CompucolorIIKey::Three;
        case sf::Keyboard::Key::Num4:
            return Compucolor::Keyboard::CompucolorIIKey::Four;
        case sf::Keyboard::Key::Num5:
            return Compucolor::Keyboard::CompucolorIIKey::Five;
        case sf::Keyboard::Key::Num6:
            return Compucolor::Keyboard::CompucolorIIKey::Six;
        case sf::Keyboard::Key::Num7:
            return Compucolor::Keyboard::CompucolorIIKey::Seven;
        case sf::Keyboard::Key::Num8:
            return Compucolor::Keyboard::CompucolorIIKey::Eight;
        case sf::Keyboard::Key::Num9:
            return Compucolor::Keyboard::CompucolorIIKey::Nine;
        case sf::Keyboard::Key::Numpad0:
            return Compucolor::Keyboard::CompucolorIIKey::Num0;
        case sf::Keyboard::Key::Numpad1:
            return Compucolor::Keyboard::CompucolorIIKey::Num1;
        case sf::Keyboard::Key::Numpad2:
            return Compucolor::Keyboard::CompucolorIIKey::Num2;
        case sf::Keyboard::Key::Numpad3:
            return Compucolor::Keyboard::CompucolorIIKey::Num3;
        case sf::Keyboard::Key::Numpad4:
            return Compucolor::Keyboard::CompucolorIIKey::Num4;
        case sf::Keyboard::Key::Numpad5:
            return Compucolor::Keyboard::CompucolorIIKey::Num5;
        case sf::Keyboard::Key::Numpad6:
            return Compucolor::Keyboard::CompucolorIIKey::Num6;
        case sf::Keyboard::Key::Numpad7:
            return Compucolor::Keyboard::CompucolorIIKey::Num7;
        case sf::Keyboard::Key::Numpad8:
            return Compucolor::Keyboard::CompucolorIIKey::Num8;
        case sf::Keyboard::Key::Numpad9:
            return Compucolor::Keyboard::CompucolorIIKey::Num9;
        case sf::Keyboard::Key::F1:
            return Compucolor::Keyboard::CompucolorIIKey::F1;
        case sf::Keyboard::Key::F2:
            return Compucolor::Keyboard::CompucolorIIKey::F2;
        case sf::Keyboard::Key::F3:
            return Compucolor::Keyboard::CompucolorIIKey::F3;
        case sf::Keyboard::Key::F4:
            return Compucolor::Keyboard::CompucolorIIKey::F4;
        case sf::Keyboard::Key::F5:
            return Compucolor::Keyboard::CompucolorIIKey::F5;
        case sf::Keyboard::Key::F6:
            return Compucolor::Keyboard::CompucolorIIKey::F6;
        case sf::Keyboard::Key::F7:
            return Compucolor::Keyboard::CompucolorIIKey::F7;
        case sf::Keyboard::Key::F8:
            return Compucolor::Keyboard::CompucolorIIKey::F8;
        case sf::Keyboard::Key::F9:
            return Compucolor::Keyboard::CompucolorIIKey::F9;
        case sf::Keyboard::Key::F10:
            return Compucolor::Keyboard::CompucolorIIKey::F10;
        case sf::Keyboard::Key::F11:
            return Compucolor::Keyboard::CompucolorIIKey::F11;
        case sf::Keyboard::Key::F12:
            return Compucolor::Keyboard::CompucolorIIKey::F12;
        case sf::Keyboard::Key::F13:
            return Compucolor::Keyboard::CompucolorIIKey::F13;
        case sf::Keyboard::Key::F14:
            return Compucolor::Keyboard::CompucolorIIKey::F14;
        case sf::Keyboard::Key::F15:
            return Compucolor::Keyboard::CompucolorIIKey::F15;
        case sf::Keyboard::Key::Escape:
            return Compucolor::Keyboard::CompucolorIIKey::Esc;
        case sf::Keyboard::Key::LControl:
            return Compucolor::Keyboard::CompucolorIIKey::Ctrl;
        case sf::Keyboard::Key::LShift:
            return Compucolor::Keyboard::CompucolorIIKey::Shift;
        case sf::Keyboard::Key::LAlt:
            return Compucolor::Keyboard::CompucolorIIKey::Repeat;
        case sf::Keyboard::Key::LSystem:
            return Compucolor::Keyboard::CompucolorIIKey::Repeat;
        case sf::Keyboard::Key::RControl:
            return Compucolor::Keyboard::CompucolorIIKey::Ctrl;
        case sf::Keyboard::Key::RShift:
            return Compucolor::Keyboard::CompucolorIIKey::Shift;
        case sf::Keyboard::Key::RAlt:
            return Compucolor::Keyboard::CompucolorIIKey::Repeat;
        case sf::Keyboard::Key::RSystem:
            return Compucolor::Keyboard::CompucolorIIKey::Repeat;
        case sf::Keyboard::Key::Space:
            return Compucolor::Keyboard::CompucolorIIKey::Space;
        case sf::Keyboard::Key::Backspace:
            return Compucolor::Keyboard::CompucolorIIKey::Bksp;
        case sf::Keyboard::Key::Enter:
            return Compucolor::Keyboard::CompucolorIIKey::Cr;
        case sf::Keyboard::Key::Left:
            return Compucolor::Keyboard::CompucolorIIKey::Curlft;
        case sf::Keyboard::Key::Right:
            return Compucolor::Keyboard::CompucolorIIKey::Currgt;
        case sf::Keyboard::Key::Up:
            return Compucolor::Keyboard::CompucolorIIKey::CurUp;
        case sf::Keyboard::Key::Down:
            return Compucolor::Keyboard::CompucolorIIKey::CurDwn;
        case sf::Keyboard::Key::Home:
            return Compucolor::Keyboard::CompucolorIIKey::Home;
        case sf::Keyboard::Key::Tab:
            return Compucolor::Keyboard::CompucolorIIKey::Tab;
        case sf::Keyboard::Key::Insert:
            return Compucolor::Keyboard::CompucolorIIKey::Inschar;
        case sf::Keyboard::Key::Delete:
            return Compucolor::Keyboard::CompucolorIIKey::Delchar;
        case sf::Keyboard::Key::PageUp:
            return Compucolor::Keyboard::CompucolorIIKey::EPage;
        case sf::Keyboard::Key::PageDown:
            return Compucolor::Keyboard::CompucolorIIKey::ELine;
        case sf::Keyboard::Key::End:
        case sf::Keyboard::Key::Pause:
            return Compucolor::Keyboard::CompucolorIIKey::Break;
        case sf::Keyboard::Key::Add:
        case sf::Keyboard::Key::Subtract:
        case sf::Keyboard::Key::Multiply:
        case sf::Keyboard::Key::Divide:
        default:
            return {};
    }
}

std::optional<char> Compucolor::Impl::App::Sfml::KeyboardTranslator::ToAscii(
    const sf::Keyboard::Key key,
    const bool shift
)
{
    using Key = sf::Keyboard::Key;

    if (key >= Key::Num0 && key <= Key::Num9)
    {
        // US layout: shifted digit row.
        static constexpr char shifted[] = {')', '!', '@', '#', '$', '%', '^', '&', '*', '('};
        const int digit = static_cast<int>(key) - static_cast<int>(Key::Num0);

        return shift ? shifted[digit] : static_cast<char>('0' + digit);
    }

    switch (key)
    {
        case Key::Hyphen: return shift ? '_' : '-';
        case Key::Equal: return shift ? '+' : '=';
        case Key::LBracket: return shift ? '{' : '[';
        case Key::RBracket: return shift ? '}' : ']';
        case Key::Backslash: return shift ? '|' : '\\';
        case Key::Semicolon: return shift ? ':' : ';';
        case Key::Apostrophe: return shift ? '"' : '\'';
        case Key::Comma: return shift ? '<' : ',';
        case Key::Period: return shift ? '>' : '.';
        case Key::Slash: return shift ? '?' : '/';
        case Key::Grave: return shift ? '~' : '`';
        default: return std::nullopt;
    }
}
