#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include <SFML/Graphics.hpp>

#include <Compucolor/Common/IDisplay.h>

namespace Compucolor::Impl::App::Sfml
{
    // Palettes for the eight hardware colors. This X-macro is the single source
    // of truth for enum values, ImGui combo labels and the theme count.
#define CC2_THEMES(X)                       \
    X(Standard,       "Standard")           \
    X(Synthwave,      "Synthwave")          \
    X(Gruvbox,        "Gruvbox")            \
    X(WarmRetro,      "WarmRetro")          \
    X(AmberTerminal,  "Amber Terminal")     \
    X(GreenPhosphor,  "Green Phosphor")     \
    X(Commodore64,    "Commodore 64")       \
    X(Cga,            "CGA")                \
    X(SolarizedDark,  "Solarized Dark")     \
    X(Nord,           "Nord")

    enum class Theme {
#define CC2_THEME_ENUM(name, label) name,
        CC2_THEMES(CC2_THEME_ENUM)
#undef CC2_THEME_ENUM
    };

    inline constexpr auto ThemeNames = std::to_array<const char*>({
#define CC2_THEME_LABEL(name, label) label,
        CC2_THEMES(CC2_THEME_LABEL)
#undef CC2_THEME_LABEL
    });

#undef CC2_THEMES

    [[nodiscard]] constexpr std::size_t ThemeCount() noexcept
    {
        return ThemeNames.size();
    }

    [[nodiscard]] constexpr const char* ToString(const Theme theme) noexcept
    {
        const auto index = static_cast<std::size_t>(theme);

        return index < ThemeCount() ? ThemeNames[index] : "Unknown";
    }

    // Converts a persisted index to a Theme, falling back to Standard for
    // missing or out-of-range values.
    [[nodiscard]] constexpr Theme ThemeFromInt(const int value) noexcept
    {
        if (value < 0 || value >= static_cast<int>(ThemeCount()))
        {
            return Theme::Standard;
        }

        return static_cast<Theme>(value);
    }

    class Display: public Compucolor::Common::IDisplay {
        public:
            Display(
                std::shared_ptr<sf::RenderWindow> window
            );

            void DrawPixel(Compucolor::Common::Color color, int x, int y) override;
            void Repaint() override;
            // The overlay draws after the emulator screen in window coordinates, before the buffer flip.
            void Render(const std::function<void()>& overlay = {});

            void SetSmooth(bool smooth);

            // Selects the palette used to convert hardware colors to RGBA.
            void SetTheme(Theme theme);

            // Writes the most recent complete frame as a PNG.
            bool SaveScreenshot(const std::string& path);

            void Close() override;

        private:
            static constexpr std::size_t Width = 384;
            static constexpr std::size_t Height = 256;
            static constexpr std::size_t FrameSize = Width * Height * 4;

            Theme _currentTheme;
            std::shared_ptr<sf::RenderWindow> _window;
            sf::Texture _texture;
            std::mutex _frameMutex;
            std::array<std::uint8_t, FrameSize> _backBuffer{};
            std::array<std::uint8_t, FrameSize> _publishedFrame{};
            std::atomic<bool> _isRunning;

            uint32_t GetColor(Compucolor::Common::Color color);

            static sf::View GetLetterboxView(sf::Vector2u windowSize);
    };
}