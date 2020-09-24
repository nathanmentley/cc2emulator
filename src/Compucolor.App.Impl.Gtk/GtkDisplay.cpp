#include "GtkDisplay.h"

CompucolorEmulator* buildEmulator()
{
    std::shared_ptr<IMemory> memory =
        std::shared_ptr<IMemory>(new ByteArrayMemory());

    std::shared_ptr<ICrtEmulator> crtEmulator =
        std::shared_ptr<ICrtEmulator>(new CrtEmulator(memory));

    std::shared_ptr<IIntel8080Emulator> intel8080Emulator =
        std::shared_ptr<IIntel8080Emulator>(new Intel8080Emulator(memory));

    return new CompucolorEmulator(
        memory,
        crtEmulator,
        intel8080Emulator
    );
};

GtkDisplay::GtkDisplay():
    _emulator(std::unique_ptr<CompucolorEmulator>(buildEmulator())),//get_emulator()),
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
    )
{
    gtk_window_set_title(_window.get(), "Hello there");
    gtk_window_set_default_size(_window.get(), 384, 256);

    auto halign = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(halign), GTK_WIDGET(_image.get()));
    gtk_container_add(GTK_CONTAINER(_window.get()), halign);

    g_signal_connect(_window.get(), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all((GtkWidget*)_window.get());

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
    gtk_image_set_from_pixbuf(
        _image.get(),
        _buffer.get()
    );
}

uint32_t GtkDisplay::GetColor(Color color)
{
    switch (color)
    {
        case Black:
            return 0x000000FF;
        case Red:
            return 0xFF0000FF;
        case Green:
            return 0x00FF00FF;
        case Yellow:
            return 0xFFFF00FF;
        case Blue:
            return 0x0000FFFF;
        case Purple:
            return 0xFF00FFFF;
        case Teal:
            return 0x00FFFFFF;
        case White:
            return 0xFFFFFFFF;
        default:
            return 0x000000FF;
    }
}