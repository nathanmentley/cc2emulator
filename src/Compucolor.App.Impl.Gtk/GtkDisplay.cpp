#include "GtkDisplay.h"

GtkDisplay::GtkDisplay():
    _loggerProvider(
        std::shared_ptr<ILoggerProvider>(
            new GtkLoggerProvider()
        )
    ),
    _emulator(get_emulator(_loggerProvider)),
    _window(
        std::unique_ptr<GtkWindow, GWidgetDeleter>(
            (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL)
        )
    ),
    _image(
        std::unique_ptr<GtkImage, GWidgetDeleter>(
            (GtkImage*)gtk_image_new()
        )
    ),
    _buffer(
        std::unique_ptr<GdkPixbuf, GObjectDeleter>(
            gdk_pixbuf_new(
                GDK_COLORSPACE_RGB,
                false,
                8,
                GtkDisplay::Width,
                GtkDisplay::Height
            )
        )
    ),
    _keyboard(
        std::unique_ptr<GtkKeyboard>(
            new GtkKeyboard(_emulator)
        )
    )
{
    //setup window
    gtk_window_set_title(_window.get(), "Compucolor II Emulator");
    gtk_window_set_default_size(_window.get(), 384 * 2, 256 * 2);

    //setup listeners
    g_signal_connect(_window.get(), "key-release-event", G_CALLBACK(GtkDisplay::OnKeyUp), _keyboard.get());
    g_signal_connect(_window.get(), "key_press_event", G_CALLBACK(GtkDisplay::OnKeyDown), _keyboard.get());
    g_signal_connect(_window.get(), "destroy", G_CALLBACK(GtkDisplay::QuitApp), _emulator.get());

    //setup elements
    gtk_container_add(GTK_CONTAINER(_window.get()), GTK_WIDGET(_image.get()));

    //show all
    gtk_widget_show_all(GTK_WIDGET(_window.get()));

    //start the emulator
    _emulator->SetDisplay(this);
    _emulator->Start();
}

void GtkDisplay::DrawPixel(Color color, int x, int y)
{
    std::unique_ptr<GdkPixbuf, GObjectDeleter> pixel =
        std::unique_ptr<GdkPixbuf, GObjectDeleter>(
            gdk_pixbuf_new(
                GDK_COLORSPACE_RGB,
                false,
                8,
                1,
                1
            )
        );

    gdk_pixbuf_fill(pixel.get(), GetColor(color));
    gdk_pixbuf_copy_area(pixel.get(), 0, 0, 1, 1, _buffer.get(), x, y);
}

void GtkDisplay::Repaint()
{
    std::unique_ptr<GtkAllocation> allocation =
        std::unique_ptr<GtkAllocation>(new GtkAllocation());

    gtk_widget_get_allocation(GTK_WIDGET(_image.get()), allocation.get());

    std::unique_ptr<GdkPixbuf, GObjectDeleter> pxbscaled =
        std::unique_ptr<GdkPixbuf, GObjectDeleter>(
            gdk_pixbuf_scale_simple(_buffer.get(), allocation->width, allocation->height, GDK_INTERP_BILINEAR)
        );

    gtk_image_set_from_pixbuf(_image.get(), pxbscaled.get());
}

uint32_t GtkDisplay::GetColor(Color color)
{
    switch (color)
    {
        case Color::Black:      return 0x000000FF;
        case Color::Red:        return 0xFF0000FF;
        case Color::Green:      return 0x00FF00FF;
        case Color::Yellow:     return 0xFFFF00FF;
        case Color::Blue:       return 0x0000FFFF;
        case Color::Purple:     return 0xFF00FFFF;
        case Color::Teal:       return 0x00FFFFFF;
        case Color::White:      return 0xFFFFFFFF;
        default:                return 0x000000FF;
    }
}

void GtkDisplay::QuitApp(ICompucolorEmulator* emulator)
{
    emulator->Stop();
}

void GtkDisplay::OnKeyUp(GtkWidget* widget, GdkEventKey* event, GtkKeyboard* keyboard)
{
    keyboard->OnKeyUp(event);
}

void GtkDisplay::OnKeyDown(GtkWidget* widget, GdkEventKey* event, GtkKeyboard* keyboard)
{
    keyboard->OnKeyDown(event);
}