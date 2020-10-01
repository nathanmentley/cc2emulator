#include "GtkKeyboard.h"

GtkKeyboard::GtkKeyboard(
    std::shared_ptr<ICompucolorEmulator> emulator
):
    _emulator(emulator)
{
}

void GtkKeyboard::OnKeyUp(GdkEventKey* event)
{
    std::optional<CompucolorIIKey> key = ConvertKey(event);
    
    if (key.has_value())
    {
        _emulator->OnKeyUp(key.value());
    }
}

void GtkKeyboard::OnKeyDown(GdkEventKey* event)
{
    std::optional<CompucolorIIKey> key = ConvertKey(event);

    if (key.has_value())
    {
        _emulator->OnKeyDown(key.value());
    }
}

std::optional<CompucolorIIKey> GtkKeyboard::ConvertKey(GdkEventKey* event)
{
    switch(event->keyval)
    {
        case GDK_KEY_a:         return CompucolorIIKey::A;
        case GDK_KEY_A:         return CompucolorIIKey::A;
    }

    return {};
}