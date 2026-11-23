#include "OnScreenKeyMap.h"

using Compucolor::Keyboard::CompucolorIIKey;

using Compucolor::Impl::App::Sfml::OnScreenKey;
using Compucolor::Impl::App::Sfml::OnScreenKeyRow;

namespace
{
    const std::vector<OnScreenKeyRow>& BuildRows()
    {
        static const std::vector<OnScreenKeyRow> rows = {
            {
                "Functions",
                {
                    {"F0", CompucolorIIKey::F0},
                    {"F1", CompucolorIIKey::F1},
                    {"F2", CompucolorIIKey::F2},
                    {"F3", CompucolorIIKey::F3},
                    {"F4", CompucolorIIKey::F4},
                    {"F5", CompucolorIIKey::F5},
                    {"F6", CompucolorIIKey::F6},
                    {"F7", CompucolorIIKey::F7},
                    {"F8", CompucolorIIKey::F8},
                    {"F9", CompucolorIIKey::F9},
                    {"F10", CompucolorIIKey::F10},
                    {"F11", CompucolorIIKey::F11},
                    {"F12", CompucolorIIKey::F12},
                    {"F13", CompucolorIIKey::F13},
                    {"F14", CompucolorIIKey::F14},
                    {"F15", CompucolorIIKey::F15},
                    {"BREAK", CompucolorIIKey::Break},
                    {"AUTO", CompucolorIIKey::Auto}
                }
            },
            {
                "Cursor and editing",
                {
                    {"HOME", CompucolorIIKey::Home},
                    {"TAB", CompucolorIIKey::Tab},
                    {"RETURN", CompucolorIIKey::Cr},
                    {"BKSP / LEFT", CompucolorIIKey::Bksp},
                    {"INS CHAR", CompucolorIIKey::Inschar},
                    {"DEL CHAR", CompucolorIIKey::Delchar},
                    {"INS LINE", CompucolorIIKey::Insline},
                    {"DEL LINE", CompucolorIIKey::Delline},
                    {"ERASE LINE", CompucolorIIKey::ELine},
                    {"ERASE PAGE", CompucolorIIKey::EPage},
                    {"ESC", CompucolorIIKey::Esc},
                    {"LEFT", CompucolorIIKey::Curlft},
                    {"RIGHT", CompucolorIIKey::Currgt},
                    {"UP", CompucolorIIKey::CurUp},
                    {"DOWN", CompucolorIIKey::CurDwn},
                    {"INSERT", CompucolorIIKey::Insert},
                    {"DELETE", CompucolorIIKey::Delete}
                }
            },
            {
                "Colours and display",
                {
                    {"BLACK", CompucolorIIKey::Black},
                    {"RED", CompucolorIIKey::Red},
                    {"GREEN", CompucolorIIKey::Green},
                    {"YELLOW", CompucolorIIKey::Yellow},
                    {"BLUE", CompucolorIIKey::Blue},
                    {"MAGENTA", CompucolorIIKey::Magenta},
                    {"CYAN", CompucolorIIKey::Cyan},
                    {"WHITE", CompucolorIIKey::White},
                    {"FG ON", CompucolorIIKey::FgOn},
                    {"BG ON", CompucolorIIKey::BgOn},
                    {"BLINK ON", CompucolorIIKey::BlinkOn},
                    {"A7 ON", CompucolorIIKey::A7on},
                    {"BL/A7 OFF", CompucolorIIKey::Bla7off}
                }
            },
            {
                "Typewriter",
                {
                    {"1 / !", CompucolorIIKey::One},
                    {"2 / \"", CompucolorIIKey::Two},
                    {"3 / #", CompucolorIIKey::Three},
                    {"4 / $", CompucolorIIKey::Four},
                    {"5 / %", CompucolorIIKey::Five},
                    {"6 / &", CompucolorIIKey::Six},
                    {"7 / '", CompucolorIIKey::Seven},
                    {"8 / (", CompucolorIIKey::Eight},
                    {"9 / )", CompucolorIIKey::Nine},
                    {"0", CompucolorIIKey::Zero},
                    {"- / =", CompucolorIIKey::Dash},
                    {"; / +", CompucolorIIKey::SemiColon},
                    {": / *", CompucolorIIKey::Colon},
                    {"@", CompucolorIIKey::AtSign},
                    {"[", CompucolorIIKey::BracketOpen},
                    {"\\", CompucolorIIKey::Backslash},
                    {"]", CompucolorIIKey::BracketClose},
                    {"^", CompucolorIIKey::Caret},
                    {"_", CompucolorIIKey::Underscore},
                    {", / <", CompucolorIIKey::Comma},
                    {". / >", CompucolorIIKey::Period},
                    {"/ / ?", CompucolorIIKey::Slash},
                    {"SPACE", CompucolorIIKey::Space}
                }
            },
            {
                "Letters",
                {
                    {"A / a", CompucolorIIKey::A},
                    {"B / b", CompucolorIIKey::B},
                    {"C / c", CompucolorIIKey::C},
                    {"D / d", CompucolorIIKey::D},
                    {"E / e", CompucolorIIKey::E},
                    {"F / f", CompucolorIIKey::F},
                    {"G / g", CompucolorIIKey::G},
                    {"H / h", CompucolorIIKey::H},
                    {"I / i", CompucolorIIKey::I},
                    {"J / j", CompucolorIIKey::J},
                    {"K / k", CompucolorIIKey::K},
                    {"L / l", CompucolorIIKey::L},
                    {"M / m", CompucolorIIKey::M},
                    {"N / n", CompucolorIIKey::N},
                    {"O / o", CompucolorIIKey::O},
                    {"P / p", CompucolorIIKey::P},
                    {"Q / q", CompucolorIIKey::Q},
                    {"R / r", CompucolorIIKey::R},
                    {"S / s", CompucolorIIKey::S},
                    {"T / t", CompucolorIIKey::T},
                    {"U / u", CompucolorIIKey::U},
                    {"V / v", CompucolorIIKey::V},
                    {"W / w", CompucolorIIKey::W},
                    {"X / x", CompucolorIIKey::X},
                    {"Y / y", CompucolorIIKey::Y},
                    {"Z / z", CompucolorIIKey::Z}
                }
            },
            {
                "Modifiers (click to latch)",
                {
                    {"SHIFT", CompucolorIIKey::Shift},
                    {"CTRL", CompucolorIIKey::Ctrl},
                    {"CAPS LOCK", CompucolorIIKey::Capslock},
                    {"REPEAT", CompucolorIIKey::Repeat}
                }
            },
            {
                "Numpad",
                {
                    {"0", CompucolorIIKey::Num0},
                    {"1", CompucolorIIKey::Num1},
                    {"2", CompucolorIIKey::Num2},
                    {"3", CompucolorIIKey::Num3},
                    {"4", CompucolorIIKey::Num4},
                    {"5", CompucolorIIKey::Num5},
                    {"6", CompucolorIIKey::Num6},
                    {"7", CompucolorIIKey::Num7},
                    {"8", CompucolorIIKey::Num8},
                    {"9", CompucolorIIKey::Num9},
                    {"-", CompucolorIIKey::NumDash},
                    {"/", CompucolorIIKey::NumSlash},
                    {".", CompucolorIIKey::NumPeriod}
                }
            }
        };

        return rows;
    }
}

namespace Compucolor::Impl::App::Sfml
{
    const std::vector<OnScreenKeyRow>& OnScreenKeyRows()
    {
        return BuildRows();
    }

    bool IsLatchedOnScreenModifier(const CompucolorIIKey key) noexcept
    {
        switch (key)
        {
            case CompucolorIIKey::Shift:
            case CompucolorIIKey::Ctrl:
            case CompucolorIIKey::Capslock:
            case CompucolorIIKey::Repeat:
                return true;

            default:
                return false;
        }
    }
}
