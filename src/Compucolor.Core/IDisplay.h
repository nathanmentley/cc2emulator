#pragma once

#include "Color.h"

class IDisplay {
    public:
        virtual void DrawPixel(Color color, int x, int y) = 0;
        virtual void Repaint() = 0;

        virtual ~IDisplay() {}
};