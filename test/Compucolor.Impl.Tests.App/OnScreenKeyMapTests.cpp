#include <set>
#include <string_view>

#include <gtest/gtest.h>

#include "AsciiKeyMap.h"
#include "OnScreenKeyMap.h"

using Compucolor::Keyboard::CompucolorIIKey;

using Compucolor::Impl::App::Sfml::AsciiKeyMap;
using Compucolor::Impl::App::Sfml::IsLatchedOnScreenModifier;
using Compucolor::Impl::App::Sfml::OnScreenKeyRows;

namespace
{
    std::set<CompucolorIIKey> AllOnScreenKeys()
    {
        std::set<CompucolorIIKey> keys;

        for (const auto& row : OnScreenKeyRows())
        {
            for (const auto& key : row.keys)
            {
                keys.insert(key.key);
            }
        }

        return keys;
    }
}

TEST(OnScreenKeyMap, RowsHaveLabelsAndKeys) {
    //Arrange
    const auto& rows = OnScreenKeyRows();

    //Act / Assert
    ASSERT_FALSE(rows.empty());

    for (const auto& row : rows)
    {
        EXPECT_NE(row.title, nullptr);
        ASSERT_NE(row.title, std::string_view{});
        EXPECT_FALSE(row.keys.empty()) << row.title;

        for (const auto& key : row.keys)
        {
            ASSERT_NE(key.label, nullptr) << row.title;
            EXPECT_NE(key.label, std::string_view{}) << row.title;
        }
    }
}

TEST(OnScreenKeyMap, EveryCompucolorKeyAppearsExactlyOnce) {
    //Arrange
    const auto keys = AllOnScreenKeys();
    const auto last = static_cast<int>(CompucolorIIKey::NumPeriod);

    //Act / Assert
    for (int value = 0; value <= last; ++value)
    {
        EXPECT_TRUE(keys.contains(static_cast<CompucolorIIKey>(value))) << value;
    }

    std::size_t count = 0;

    for (const auto& row : OnScreenKeyRows())
    {
        count += row.keys.size();
    }

    EXPECT_EQ(count, keys.size());
    EXPECT_EQ(count, static_cast<std::size_t>(last) + 1);
}

TEST(OnScreenKeyMap, EveryPrintableAsciiKeyHasAButton) {
    //Arrange
    const auto keys = AllOnScreenKeys();

    //Act / Assert
    for (char character = ' '; character <= '~'; ++character)
    {
        const auto chord = AsciiKeyMap::Lookup(character);

        if (!chord.has_value())
        {
            continue;
        }

        EXPECT_TRUE(keys.contains(chord->key)) << character;
    }
}

TEST(OnScreenKeyMap, ModifiersAreLatched) {
    //Act / Assert
    EXPECT_TRUE(IsLatchedOnScreenModifier(CompucolorIIKey::Shift));
    EXPECT_TRUE(IsLatchedOnScreenModifier(CompucolorIIKey::Ctrl));
    EXPECT_TRUE(IsLatchedOnScreenModifier(CompucolorIIKey::Capslock));
    EXPECT_TRUE(IsLatchedOnScreenModifier(CompucolorIIKey::Repeat));
    EXPECT_FALSE(IsLatchedOnScreenModifier(CompucolorIIKey::Auto));
    EXPECT_FALSE(IsLatchedOnScreenModifier(CompucolorIIKey::A));
}
