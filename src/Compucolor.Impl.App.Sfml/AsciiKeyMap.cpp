#include "AsciiKeyMap.h"

using Compucolor::Keyboard::CompucolorIIKey;

using Compucolor::Impl::App::Sfml::AsciiKeyMap;
using Compucolor::Impl::App::Sfml::KeyChord;

std::optional<KeyChord> AsciiKeyMap::Lookup(const char character)
{
    if (character >= 'a' && character <= 'z')
    {
        return Lookup(static_cast<char>(character - 'a' + 'A'));
    }

    if (character >= 'A' && character <= 'Z')
    {
        const int offset = character - 'A';

        return KeyChord{static_cast<CompucolorIIKey>(static_cast<int>(CompucolorIIKey::A) + offset), false};
    }

    if (character >= '0' && character <= '9')
    {
        const int offset = character - '0';

        return KeyChord{static_cast<CompucolorIIKey>(static_cast<int>(CompucolorIIKey::Zero) + offset), false};
    }

    // Shifted digits and punctuation differ from a PC layout, so each one is listed.
    switch (character)
    {
        case ' ': return KeyChord{CompucolorIIKey::Space, false};
        case '\n':
        case '\r': return KeyChord{CompucolorIIKey::Cr, false};
        case '\t': return KeyChord{CompucolorIIKey::Tab, false};

        case '!': return KeyChord{CompucolorIIKey::One, true};
        case '"': return KeyChord{CompucolorIIKey::Two, true};
        case '#': return KeyChord{CompucolorIIKey::Three, true};
        case '$': return KeyChord{CompucolorIIKey::Four, true};
        case '%': return KeyChord{CompucolorIIKey::Five, true};
        case '&': return KeyChord{CompucolorIIKey::Six, true};
        case '\'': return KeyChord{CompucolorIIKey::Seven, true};
        case '(': return KeyChord{CompucolorIIKey::Eight, true};
        case ')': return KeyChord{CompucolorIIKey::Nine, true};
        case '*': return KeyChord{CompucolorIIKey::Colon, true};
        case '+': return KeyChord{CompucolorIIKey::SemiColon, true};

        case ',': return KeyChord{CompucolorIIKey::Comma, false};
        case '-': return KeyChord{CompucolorIIKey::Dash, false};
        case '.': return KeyChord{CompucolorIIKey::Period, false};
        case '/': return KeyChord{CompucolorIIKey::Slash, false};
        case ':': return KeyChord{CompucolorIIKey::Colon, false};
        case ';': return KeyChord{CompucolorIIKey::SemiColon, false};

        case '<': return KeyChord{CompucolorIIKey::Comma, true};
        case '=': return KeyChord{CompucolorIIKey::Dash, true};
        case '>': return KeyChord{CompucolorIIKey::Period, true};
        case '?': return KeyChord{CompucolorIIKey::Slash, true};

        case '@': return KeyChord{CompucolorIIKey::AtSign, false};
        case '[': return KeyChord{CompucolorIIKey::BracketOpen, false};
        case '\\': return KeyChord{CompucolorIIKey::Backslash, false};
        case ']': return KeyChord{CompucolorIIKey::BracketClose, false};
        case '^': return KeyChord{CompucolorIIKey::Caret, false};
        case '_': return KeyChord{CompucolorIIKey::Underscore, false};

        default: return std::nullopt;
    }
}
