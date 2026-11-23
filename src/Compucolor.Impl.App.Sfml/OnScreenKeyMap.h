#pragma once

#include <vector>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

namespace Compucolor::Impl::App::Sfml
{
    // A clickable key in the on-screen keyboard panel. Labels show the
    // unshifted and shifted characters where the CC2 prints both.
    struct OnScreenKey {
        const char* label;
        Compucolor::Keyboard::CompucolorIIKey key;
    };

    struct OnScreenKeyRow {
        const char* title;
        std::vector<OnScreenKey> keys;
    };

    // Grouped layout covering the full Compucolor II key enum. Sorted by
    // function rather than by physical key so the panel stays readable.
    [[nodiscard]] const std::vector<OnScreenKeyRow>& OnScreenKeyRows();

    // These row-16 keys are held until the user clicks them again, so a
    // following key click is sampled with the modifier active.
    [[nodiscard]] bool IsLatchedOnScreenModifier(
        Compucolor::Keyboard::CompucolorIIKey key
    ) noexcept;
}
