#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include <SFML/Graphics.hpp>

#include <Compucolor/Common/IDisplay.h>

namespace Compucolor::Impl::App::Sfml
{
    enum class Theme {
        Standard = 0,
        Synthwave = 1,
        Gruvbox = 2,
        WarmRetro = 3
    };

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