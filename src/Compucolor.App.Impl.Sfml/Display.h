#pragma once

#include <memory>

#include <SFML/Graphics.hpp>

#include <Compucolor/Common/IDisplay.h>

namespace Compucolor::App::Impl::Sfml
{
    class Display: public Common::IDisplay {
        public:
            Display(
                std::shared_ptr<sf::RenderWindow> window
            );

            virtual void DrawPixel(Common::Color color, int x, int y) override;
            virtual void Repaint() override;

        private:
            std::shared_ptr<sf::RenderWindow> _window;

            static uint32_t GetColor(Common::Color color);
    };
}