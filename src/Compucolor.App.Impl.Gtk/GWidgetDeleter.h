#pragma once

#include <gtk/gtk.h>

struct GWidgetDeleter {
    void operator()(void* ptr) {
        gtk_widget_destroy((GtkWidget*)ptr);
    }
};