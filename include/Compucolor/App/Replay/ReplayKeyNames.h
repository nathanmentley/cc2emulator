#pragma once

#include <cstddef>
#include <optional>
#include <string>

#include <Compucolor/Keyboard/CompucolorIIKey.h>

// Single source of truth for replay key names. Every CompucolorIIKey must be
// listed here so bundle parsing and round-trip tests cannot drift apart.
#define CC2_REPLAY_KEYS(CC2_KEY) \
    CC2_KEY(Zero) \
    CC2_KEY(One) \
    CC2_KEY(Two) \
    CC2_KEY(Three) \
    CC2_KEY(Four) \
    CC2_KEY(Five) \
    CC2_KEY(Six) \
    CC2_KEY(Seven) \
    CC2_KEY(Eight) \
    CC2_KEY(Nine) \
    CC2_KEY(SemiColon) \
    CC2_KEY(Colon) \
    CC2_KEY(Comma) \
    CC2_KEY(Dash) \
    CC2_KEY(Period) \
    CC2_KEY(Slash) \
    CC2_KEY(AtSign) \
    CC2_KEY(A) \
    CC2_KEY(B) \
    CC2_KEY(C) \
    CC2_KEY(D) \
    CC2_KEY(E) \
    CC2_KEY(F) \
    CC2_KEY(G) \
    CC2_KEY(H) \
    CC2_KEY(I) \
    CC2_KEY(J) \
    CC2_KEY(K) \
    CC2_KEY(L) \
    CC2_KEY(M) \
    CC2_KEY(N) \
    CC2_KEY(O) \
    CC2_KEY(P) \
    CC2_KEY(Q) \
    CC2_KEY(R) \
    CC2_KEY(S) \
    CC2_KEY(T) \
    CC2_KEY(U) \
    CC2_KEY(V) \
    CC2_KEY(W) \
    CC2_KEY(X) \
    CC2_KEY(Y) \
    CC2_KEY(Z) \
    CC2_KEY(BracketOpen) \
    CC2_KEY(Backslash) \
    CC2_KEY(BracketClose) \
    CC2_KEY(Caret) \
    CC2_KEY(Underscore) \
    CC2_KEY(F0) \
    CC2_KEY(F1) \
    CC2_KEY(F2) \
    CC2_KEY(F3) \
    CC2_KEY(F4) \
    CC2_KEY(F5) \
    CC2_KEY(F6) \
    CC2_KEY(F7) \
    CC2_KEY(F8) \
    CC2_KEY(F9) \
    CC2_KEY(F10) \
    CC2_KEY(F11) \
    CC2_KEY(F12) \
    CC2_KEY(F13) \
    CC2_KEY(F14) \
    CC2_KEY(F15) \
    CC2_KEY(Break) \
    CC2_KEY(Inschar) \
    CC2_KEY(Delline) \
    CC2_KEY(Insline) \
    CC2_KEY(Delchar) \
    CC2_KEY(Auto) \
    CC2_KEY(Home) \
    CC2_KEY(Tab) \
    CC2_KEY(CurDwn) \
    CC2_KEY(ELine) \
    CC2_KEY(EPage) \
    CC2_KEY(Cr) \
    CC2_KEY(A7on) \
    CC2_KEY(Bla7off) \
    CC2_KEY(Black) \
    CC2_KEY(Red) \
    CC2_KEY(Green) \
    CC2_KEY(Yellow) \
    CC2_KEY(Blue) \
    CC2_KEY(Magenta) \
    CC2_KEY(Cyan) \
    CC2_KEY(White) \
    CC2_KEY(Currgt) \
    CC2_KEY(Curlft) \
    CC2_KEY(Esc) \
    CC2_KEY(CurUp) \
    CC2_KEY(FgOn) \
    CC2_KEY(BgOn) \
    CC2_KEY(BlinkOn) \
    CC2_KEY(Space) \
    CC2_KEY(Capslock) \
    CC2_KEY(Repeat) \
    CC2_KEY(Shift) \
    CC2_KEY(Ctrl) \
    CC2_KEY(Bksp) \
    CC2_KEY(Delete) \
    CC2_KEY(Insert) \
    CC2_KEY(Num0) \
    CC2_KEY(Num1) \
    CC2_KEY(Num2) \
    CC2_KEY(Num3) \
    CC2_KEY(Num4) \
    CC2_KEY(Num5) \
    CC2_KEY(Num6) \
    CC2_KEY(Num7) \
    CC2_KEY(Num8) \
    CC2_KEY(Num9) \
    CC2_KEY(NumDash) \
    CC2_KEY(NumSlash) \
    CC2_KEY(NumPeriod)

namespace Compucolor::Impl::App::Replay
{
    class ReplayKeyNames {
        public:
            [[nodiscard]] static std::string ToString(
                Compucolor::Keyboard::CompucolorIIKey key
            );

            [[nodiscard]] static std::optional<Compucolor::Keyboard::CompucolorIIKey>
            FromString(const std::string& name);

            [[nodiscard]] static constexpr std::size_t Count()
            {
                return KeyCount;
            }

        private:
            static constexpr std::size_t KeyCount = [] {
                return 0
#define CC2_COUNT(name) + 1
                    CC2_REPLAY_KEYS(CC2_COUNT)
#undef CC2_COUNT
                    ;
            }();
    };
}
