#pragma once

 /* this is the character generator ROM for the character mode of a
* compucolor ii computer.  the character cell is 8 rows of six pixels.
* for the most part, characters occuply 5x7 of that 6x8 cell.
*/

#include <memory>

namespace Compucolor::Crt::Impl
{
    uint8_t* get_uf6_rom();
    uint8_t* get_uf6_rom_lowercase();
}