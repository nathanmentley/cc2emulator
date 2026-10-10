#pragma once

#include <optional>
#include <vector>

#include <Compucolor/Common/IDisplay.h>

// Records the last color drawn at each pixel of the 384x256 Compucolor screen.
class FakeDisplay: public Compucolor::Common::IDisplay
{
    public:
        static constexpr int Width = 384;
        static constexpr int Height = 256;

        FakeDisplay(): _pixels(Width * Height) {}

        void DrawPixel(Compucolor::Common::Color color, int x, int y) override
        {
            _pixels[y * Width + x] = color;
        }

        void Repaint() override { repaints++; }

        void Close() override {}

        [[nodiscard]] std::optional<Compucolor::Common::Color> PixelAt(int x, int y) const
        {
            return _pixels[y * Width + x];
        }

        int repaints = 0;

    private:
        std::vector<std::optional<Compucolor::Common::Color>> _pixels;
};
