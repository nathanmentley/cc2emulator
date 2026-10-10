#pragma once

#include <optional>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

namespace Compucolor::Impl::App::Sfml
{
    struct KeyChord {
        Compucolor::Keyboard::CompucolorIIKey key;
        bool shift;
    };

    // Maps printable ASCII to the Compucolor key and shift state that produce it.
    class AsciiKeyMap {
        public:
            // Lowercase letters map to uppercase because the standard character set has no lowercase.
            static std::optional<KeyChord> Lookup(char character);
    };
}
