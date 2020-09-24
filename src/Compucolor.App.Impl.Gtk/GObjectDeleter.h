#pragma once

#include <gtk/gtk.h>

struct GObjectDeleter {
    void operator()(void* ptr) {
        g_object_unref(ptr);
    }
};