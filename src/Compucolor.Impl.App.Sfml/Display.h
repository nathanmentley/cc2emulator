#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>

#include <SFML/Graphics.hpp>

#include <Compucolor/Common/IDisplay.h>

namespace Compucolor::Impl::App::Sfml
{
    class Display: public Compucolor::Common::IDisplay {
        public:
            Display(
                std::shared_ptr<sf::RenderWindow> window
            );

            void DrawPixel(Compucolor::Common::Color color, int x, int y) override;
            void Repaint() override;
            void Render();

            void Close() override;

        private:
            static constexpr std::size_t Width = 384;
            static constexpr std::size_t Height = 256;
            static constexpr std::size_t FrameSize = Width * Height * 4;

            std::shared_ptr<sf::RenderWindow> _window;
            sf::Texture _texture;
            std::mutex _frameMutex;
            std::array<std::uint8_t, FrameSize> _backBuffer{};
            std::array<std::uint8_t, FrameSize> _publishedFrame{};
            std::atomic<bool> _isRunning;

            static uint32_t GetColor(Compucolor::Common::Color color);
    };
}