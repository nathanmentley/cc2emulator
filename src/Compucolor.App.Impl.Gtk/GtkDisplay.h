#pragma once

#include <memory>

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <Compucolor.App/get_emulator.h>
#include <Compucolor/Common/IDisplay.h>

#include <Compucolor.App.Impl.Gtk/GtkLoggerProvider.h>
#include <Compucolor.App.Impl.Gtk/GtkKeyboard.h>
#include <Compucolor.App.Impl.Gtk/GObjectDeleter.h>
#include <Compucolor.App.Impl.Gtk/GWidgetDeleter.h>

class GtkDisplay: public IDisplay {
    public:
        GtkDisplay();

        virtual void DrawPixel(Color color, int x, int y) override;
        virtual void Repaint() override;

    private:
        static int const Columns = 64;
        static int const Rows = 32;
        static int const CharacterWidth = 6;
        static int const CharacterHeight = 8;
        static int const Width = Columns * CharacterWidth;
        static int const Height = Rows * CharacterHeight;

        std::shared_ptr<ILoggerProvider> _loggerProvider;
        std::shared_ptr<ICompucolorEmulator> _emulator;
        std::unique_ptr<GtkWindow, GWidgetDeleter> _window;
        std::unique_ptr<GtkImage, GWidgetDeleter> _image;
        std::unique_ptr<GdkPixbuf, GObjectDeleter> _buffer;
        std::unique_ptr<GtkKeyboard> _keyboard;

        static uint32_t GetColor(Color color);

        static void QuitApp(ICompucolorEmulator* emulator);
        static void OnKeyUp(GtkWidget* widget, GdkEventKey* event, GtkKeyboard* keyboard);
        static void OnKeyDown(GtkWidget* widget, GdkEventKey* event, GtkKeyboard* keyboard);
};