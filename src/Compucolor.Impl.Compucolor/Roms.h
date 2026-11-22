
#pragma once

#include <cstdint>

#include <memory>

namespace Compucolor::Impl::CompucolorII
{
    // ROM images embedded in Roms.cpp; the files under data/roms/ are not read at runtime.
    uint8_t* get_system_rom_6_78();
    uint16_t get_system_rom_6_78_length();

    uint8_t* get_system_rom_8_79();
    uint16_t get_system_rom_8_79_length();

    // 32-byte color/timing PROM read on I/O ports 0x80-0x9F.
    uint8_t* get_crt_timing_rom();
}
