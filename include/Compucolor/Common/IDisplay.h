#pragma once

#include <Compucolor/Common/Color.h>

namespace Compucolor::Common
{
    class IDisplay {
        public:
            virtual void DrawPixel(Color color, int x, int y) = 0;
            virtual void Repaint() = 0;
            virtual void Close() = 0;

            virtual ~IDisplay() = default;
    };
}