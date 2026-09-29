#include <array>
#include <memory>

#include <gtest/gtest.h>

#include "CompucolorIntel8080Bus.h"

TEST(CompucolorIntel8080Bus, ReadsCrtTimingProm)
{
    constexpr std::array<uint8_t, 7> expected = {
        0x5C, 0x79, 0x3B, 0x9F, 0x06, 0x0C, 0x1F
    };
    Compucolor::Impl::CompucolorII::CompucolorIntel8080Bus bus(nullptr, nullptr);

    for (uint8_t address = 0; address < expected.size(); ++address)
    {
        EXPECT_EQ(expected[address], bus.Read(static_cast<uint8_t>(0x80 + address)));
    }
}
