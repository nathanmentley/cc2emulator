#pragma once

#include <cstdint>
#include <string>

namespace Compucolor::Impl::App::Sfml
{
    enum class DebugPortDirection {
        Any,
        Read,
        Write
    };

    // Configuration for one event stream in DebugMonitor. Filters are applied
    // at recording time so a trigger can stop a noisy stream from filling the
    // bounded buffers.
    struct DebugFilter {
        bool enabled = false;
        bool stopOnMatch = false;

        bool filterByPc = false;
        uint16_t pcFrom = 0x0000;
        uint16_t pcTo = 0xFFFF;

        uint8_t portFrom = 0x00;
        uint8_t portTo = 0xFF;
        bool filterByPort = false;
        DebugPortDirection direction = DebugPortDirection::Any;

        // Substring of the disassembled mnemonic, case-insensitive.
        std::string text;
    };
}
