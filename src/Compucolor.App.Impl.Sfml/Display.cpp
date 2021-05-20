#include <Compucolor.App.Impl.Sfml/Display.h>

Compucolor::App::Impl::Smfl::Display::Display(
    std::shared_ptr<sf::RenderWindow> window
):
    _window(window)
{
}

void Compucolor::App::Impl::Smfl::Display::DrawPixel(Common::Color color, int x, int y)
{
    //TODO: Buffer this, and draw the collection of pixels at the end.
    //  That'll be much quicker, and allow for simple retro effects to be added.

    sf::RectangleShape rect = sf::RectangleShape({ 1, 1 });
    rect.setPosition({static_cast<float>(x), static_cast<float>(y)});
    rect.setFillColor(sf::Color(GetColor(color)));

    _window->draw(rect);
}

void Compucolor::App::Impl::Smfl::Display::Repaint()
{
    _window->display();
    _window->clear();
}

uint32_t Compucolor::App::Impl::Smfl::Display::GetColor(Common::Color color)
{
    switch (color)
    {
        case Common::Color::Black:  return 0x000000FF;
        case Common::Color::Red:    return 0xFF0000FF;
        case Common::Color::Green:  return 0x00FF00FF;
        case Common::Color::Yellow: return 0xFFFF00FF;
        case Common::Color::Blue:   return 0x0000FFFF;
        case Common::Color::Purple: return 0xFF00FFFF;
        case Common::Color::Teal:   return 0x00FFFFFF;
        case Common::Color::White:  return 0xFFFFFFFF;
        default:                    return 0x000000FF;
    }
}