#pragma once

#include "Color.h"

namespace Compucolor::Common
{
    class IDisplay {
        public:
            virtual void DrawPixel(Compucolor::Common::Color color, int x, int y) = 0;
            virtual void Repaint() = 0;

            virtual ~IDisplay() {}
    };
}