
#pragma once

#include <cstdint>

#include <memory>

namespace Compucolor::Impl::CompucolorII
{
    uint8_t* get_system_rom_6_78();
    uint16_t get_system_rom_6_78_length();

    uint8_t* get_system_rom_8_79();
    uint16_t get_system_rom_8_79_length();

    uint8_t* get_crt_timing_rom();
}