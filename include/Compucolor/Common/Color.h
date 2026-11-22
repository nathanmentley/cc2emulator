#pragma once

namespace Compucolor::Common
{
    // The Compucolor II's eight hardware colors, addressed by a 3-bit value
    // in each character's attribute byte. RGB values depend on the display palette.
    enum class Color {
        Black = 0,
        Red = 1,
        Green = 2,
        Yellow = 3,
        Blue = 4,
        Purple = 5,
        Teal = 6,
        White = 7
    };
}
