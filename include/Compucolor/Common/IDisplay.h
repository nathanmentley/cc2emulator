#pragma once

#include <Compucolor/Common/Color.h>

namespace Compucolor::Common
{
    // Host-provided framebuffer used by the CRT emulator. DrawPixel is called
    // from the CRT thread; Repaint publishes a completed frame; Close stops
    // further drawing during shutdown.
    class IDisplay {
        public:
            virtual void DrawPixel(Color color, int x, int y) = 0;

            // Publishes the back buffer for presentation by the UI thread.
            virtual void Repaint() = 0;

            virtual void Close() = 0;

            virtual ~IDisplay() = default;
    };
}
