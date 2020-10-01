#pragma once

#include <memory>
#include <optional>

#include <gtk/gtk.h>

#include "Compucolor.Compucolor/ICompucolorEmulator.h"

class GtkKeyboard {
    public:
        GtkKeyboard(
            std::shared_ptr<ICompucolorEmulator> emulator
        );

        void OnKeyUp(GdkEventKey* event);
        void OnKeyDown(GdkEventKey* event);

    private:
        std::shared_ptr<ICompucolorEmulator> _emulator;

        std::optional<CompucolorIIKey> ConvertKey(GdkEventKey* event);
};