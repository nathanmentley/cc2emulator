#pragma once

namespace Compucolor
{
    // Supported boot ROMs. v6.78 uses a 3-phase floppy stepper; v8.79 a 4-phase one.
    enum class RomVersion {
        V6_78,
        V8_79
    };
}
