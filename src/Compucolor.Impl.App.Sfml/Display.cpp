#include "Display.h"

#include <utility>

using Compucolor::Common::Color;

using Compucolor::Impl::App::Sfml::Display;

Display::Display(std::shared_ptr<sf::RenderWindow> window):
    _window(std::move(window)),
    _texture(sf::Vector2u{Width, Height}),
    _isRunning(true)
{
    for (std::size_t alphaIndex = 3; alphaIndex < FrameSize; alphaIndex += 4)
    {
        _backBuffer[alphaIndex] = 0xFF;
        _publishedFrame[alphaIndex] = 0xFF;
    }
}

void Display::DrawPixel(const Color color, const int x, const int y)
{
    if (!_isRunning.load(std::memory_order_relaxed) || x < 0 || y < 0 || x >= Width || y >= Height) {
        return;
    }

    const std::size_t pixelIndex = (static_cast<std::size_t>(y) * Width + static_cast<std::size_t>(x)) * 4;
    const uint32_t rgba = GetColor(color);
    _backBuffer[pixelIndex] = static_cast<std::uint8_t>(rgba >> 24);
    _backBuffer[pixelIndex + 1] = static_cast<std::uint8_t>(rgba >> 16);
    _backBuffer[pixelIndex + 2] = static_cast<std::uint8_t>(rgba >> 8);
    _backBuffer[pixelIndex + 3] = static_cast<std::uint8_t>(rgba);
}

void Display::Repaint()
{
    if (!_isRunning.load(std::memory_order_relaxed)) {
        return;
    }

    std::lock_guard lock(_frameMutex);
    _backBuffer.swap(_publishedFrame);
}

void Display::Render()
{
    if (!_isRunning.load(std::memory_order_relaxed) || !_window->isOpen()) {
        return;
    }

    {
        std::lock_guard lock(_frameMutex);
        _texture.update(_publishedFrame.data());
    }

    _window->clear(sf::Color::Black);
    _window->draw(sf::Sprite(_texture));
    _window->display();
}

void Display::Close()
{
    _isRunning.store(false, std::memory_order_relaxed);
}

uint32_t Display::GetColor(const Color color)
{
    switch (color)
    {
        case Color::Black:  return 0x000000FF;
        case Color::Red:    return 0xFF0000FF;
        case Color::Green:  return 0x00FF00FF;
        case Color::Yellow: return 0xFFFF00FF;
        case Color::Blue:   return 0x0000FFFF;
        case Color::Purple: return 0xFF00FFFF;
        case Color::Teal:   return 0x00FFFFFF;
        case Color::White:  return 0xFFFFFFFF;
        default:            return 0x000000FF;
    }
}