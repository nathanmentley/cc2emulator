#pragma once

namespace Compucolor::Common
{
    // Marker base for interfaces that can be loaded from a plugin library.
    // Every plugin exports a "create" factory returning a shared_ptr to one of these.
    class IPlugin {};
}
