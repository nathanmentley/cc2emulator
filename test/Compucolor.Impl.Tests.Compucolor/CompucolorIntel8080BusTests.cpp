#include <array>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CompucolorIntel8080Bus.h"

#include <Compucolor.Tests.Core/MockDebugSink.h>

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Truly;

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

TEST(CompucolorIntel8080Bus, RecordsPortAccessesWhenEnabled)
{
    auto sink = std::make_shared<NiceMock<MockDebugSink>>();
    ON_CALL(*sink, IsEnabled()).WillByDefault(Return(true));
    ON_CALL(*sink, GetCurrentPc()).WillByDefault(Return(0x1234));

    EXPECT_CALL(
        *sink,
        OnPort(Truly([](const Compucolor::Debug::PortEvent& event) {
            return !event.isWrite &&
                event.pc == 0x1234 &&
                event.port == 0x42 &&
                event.value == 0x42;
        }))
    ).Times(1);

    Compucolor::Impl::CompucolorII::CompucolorIntel8080Bus bus(nullptr, nullptr);
    bus.SetDebugSink(sink);

    EXPECT_EQ(0x42, bus.Read(0x42));
}
