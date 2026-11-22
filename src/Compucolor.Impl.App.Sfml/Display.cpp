#include "Display.h"

#include <utility>

using Compucolor::Common::Color;

using Compucolor::Impl::App::Sfml::Display;

Display::Display(std::shared_ptr<sf::RenderWindow> window):
    _currentTheme(Theme::Standard),
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

void Display::Render(const std::function<void()>& overlay)
{
    if (!_isRunning.load(std::memory_order_relaxed) || !_window->isOpen()) {
        return;
    }

    {
        std::lock_guard lock(_frameMutex);
        _texture.update(_publishedFrame.data());
    }

    _window->clear(sf::Color::Black);
    _window->setView(GetLetterboxView(_window->getSize()));
    _window->draw(sf::Sprite(_texture));

    if (overlay)
    {
        _window->setView(_window->getDefaultView());
        overlay();
    }

    _window->display();
}

void Display::SetSmooth(const bool smooth)
{
    _texture.setSmooth(smooth);
}

void Display::SetTheme(const Theme theme)
{
    _currentTheme = theme;
}

bool Display::SaveScreenshot(const std::string& path)
{
    std::lock_guard lock(_frameMutex);

    const sf::Image image(sf::Vector2u{Width, Height}, _publishedFrame.data());

    return image.saveToFile(path);
}

sf::View Display::GetLetterboxView(const sf::Vector2u windowSize)
{
    sf::View view(sf::FloatRect({0.f, 0.f}, {static_cast<float>(Width), static_cast<float>(Height)}));

    if (windowSize.x == 0 || windowSize.y == 0)
    {
        return view;
    }

    const float targetAspect = static_cast<float>(Width) / static_cast<float>(Height);
    const float windowAspect = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);

    sf::FloatRect viewport({0.f, 0.f}, {1.f, 1.f});

    if (windowAspect > targetAspect)
    {
        viewport.size.x = targetAspect / windowAspect;
        viewport.position.x = (1.f - viewport.size.x) / 2.f;
    }
    else
    {
        viewport.size.y = windowAspect / targetAspect;
        viewport.position.y = (1.f - viewport.size.y) / 2.f;
    }

    view.setViewport(viewport);

    return view;
}

void Display::Close()
{
    _isRunning.store(false, std::memory_order_relaxed);
}

uint32_t Display::GetColor(const Color color)
{
    switch (_currentTheme)
    {
        case Theme::Standard:
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
        case Theme::Synthwave:
            switch (color)
            {
                case Color::Black:  return 0x0B0221FF;
                case Color::Red:    return 0xFF2E88FF;
                case Color::Green:  return 0x39FF14FF;
                case Color::Yellow: return 0xFFD166FF;
                case Color::Blue:   return 0x6C5CE7FF;
                case Color::Purple: return 0xB026FFFF;
                case Color::Teal:   return 0x00F5D4FF;
                case Color::White:  return 0xF8F0FFFF;
                default:            return 0x0B0221FF;
            }
            break;
        case Theme::Gruvbox:
            switch (color)
            {
                case Color::Black:  return 0x282828FF;
                case Color::Red:    return 0xCC241DFF;
                case Color::Green:  return 0x98971AFF;
                case Color::Yellow: return 0xD79921FF;
                case Color::Blue:   return 0x458588FF;
                case Color::Purple: return 0xB16286FF;
                case Color::Teal:   return 0x689D6AFF;
                case Color::White:  return 0xEBDBB2FF;
                default:            return 0x282828FF;
            }
            break;
        case Theme::WarmRetro:
            switch (color)
            {
                case Color::Black:  return 0x1B1B1BFF;
                case Color::Red:    return 0xFF6B6BFF;
                case Color::Green:  return 0x7CFF6BFF;
                case Color::Yellow: return 0xFFD166FF;
                case Color::Blue:   return 0x5B8CFFFF;
                case Color::Purple: return 0xC678DDFF;
                case Color::Teal:   return 0x5EEAD4FF;
                case Color::White:  return 0xF8FAFCFF;
                default:            return 0x1B1B1BFF;
            }
            break;
        case Theme::AmberTerminal:
            switch (color)
            {
                case Color::Black:  return 0x0A0600FF;
                case Color::Red:    return 0xFF7A18FF;
                case Color::Green:  return 0xFFB000FF;
                case Color::Yellow: return 0xFFD479FF;
                case Color::Blue:   return 0x5A3200FF;
                case Color::Purple: return 0x8A4B00FF;
                case Color::Teal:   return 0xC97A00FF;
                case Color::White:  return 0xFFF1C9FF;
                default:            return 0x0A0600FF;
            }
            break;
        case Theme::GreenPhosphor:
            switch (color)
            {
                case Color::Black:  return 0x001005FF;
                case Color::Red:    return 0x4CE87AFF;
                case Color::Green:  return 0x39FF6AFF;
                case Color::Yellow: return 0xC6FF8AFF;
                case Color::Blue:   return 0x0E7A3AFF;
                case Color::Purple: return 0x2FAE64FF;
                case Color::Teal:   return 0x6BFFB0FF;
                case Color::White:  return 0xE6FFE6FF;
                default:            return 0x001005FF;
            }
            break;
        case Theme::Commodore64:
            switch (color)
            {
                case Color::Black:  return 0x000000FF;
                case Color::Red:    return 0x68372BFF;
                case Color::Green:  return 0x588D43FF;
                case Color::Yellow: return 0xB8C76FFF;
                case Color::Blue:   return 0x352879FF;
                case Color::Purple: return 0x6F3D86FF;
                case Color::Teal:   return 0x70A4B2FF;
                case Color::White:  return 0xFFFFFFFF;
                default:            return 0x000000FF;
            }
            break;
        case Theme::Cga:
            switch (color)
            {
                case Color::Black:  return 0x000000FF;
                case Color::Red:    return 0xAA0000FF;
                case Color::Green:  return 0x00AA00FF;
                case Color::Yellow: return 0xAA5500FF;
                case Color::Blue:   return 0x0000AAFF;
                case Color::Purple: return 0xAA00AAFF;
                case Color::Teal:   return 0x00AAAAFF;
                case Color::White:  return 0xAAAAAAFF;
                default:            return 0x000000FF;
            }
            break;
        case Theme::SolarizedDark:
            switch (color)
            {
                case Color::Black:  return 0x002B36FF;
                case Color::Red:    return 0xDC322FFF;
                case Color::Green:  return 0x859900FF;
                case Color::Yellow: return 0xB58900FF;
                case Color::Blue:   return 0x268BD2FF;
                case Color::Purple: return 0xD33682FF;
                case Color::Teal:   return 0x2AA198FF;
                case Color::White:  return 0xEEE8D5FF;
                default:            return 0x002B36FF;
            }
            break;
        case Theme::Nord:
            switch (color)
            {
                case Color::Black:  return 0x2E3440FF;
                case Color::Red:    return 0xBF616AFF;
                case Color::Green:  return 0xA3BE8CFF;
                case Color::Yellow: return 0xEBCB8BFF;
                case Color::Blue:   return 0x81A1C1FF;
                case Color::Purple: return 0xB48EADFF;
                case Color::Teal:   return 0x88C0D0FF;
                case Color::White:  return 0xECEFF4FF;
                default:            return 0x2E3440FF;
            }
            break;
    }
}