#include <gtest/gtest.h>

#include "AsciiKeyMap.h"

using Compucolor::Keyboard::CompucolorIIKey;
using Compucolor::Impl::App::Sfml::AsciiKeyMap;

TEST(AsciiKeyMap, UppercaseLetterIsUnshiftedLetterKey) {
    //Act
    const auto chord = AsciiKeyMap::Lookup('Q');

    //Assert
    ASSERT_TRUE(chord.has_value());
    EXPECT_EQ(CompucolorIIKey::Q, chord->key);
    EXPECT_FALSE(chord->shift);
}

TEST(AsciiKeyMap, LowercaseLetterTypesAsUppercase) {
    //Act
    const auto chord = AsciiKeyMap::Lookup('z');

    //Assert
    ASSERT_TRUE(chord.has_value());
    EXPECT_EQ(CompucolorIIKey::Z, chord->key);
    EXPECT_FALSE(chord->shift);
}

TEST(AsciiKeyMap, DigitMapsToDigitKey) {
    //Act
    const auto chord = AsciiKeyMap::Lookup('7');

    //Assert
    ASSERT_TRUE(chord.has_value());
    EXPECT_EQ(CompucolorIIKey::Seven, chord->key);
    EXPECT_FALSE(chord->shift);
}

TEST(AsciiKeyMap, ShiftedDigitSymbolsUseCompucolorLayout) {
    //Assert: on the Compucolor '&' is shift+6 and '(' is shift+8, unlike a PC.
    EXPECT_EQ(CompucolorIIKey::Six, AsciiKeyMap::Lookup('&')->key);
    EXPECT_TRUE(AsciiKeyMap::Lookup('&')->shift);
    EXPECT_EQ(CompucolorIIKey::Eight, AsciiKeyMap::Lookup('(')->key);
    EXPECT_TRUE(AsciiKeyMap::Lookup('(')->shift);
    EXPECT_EQ(CompucolorIIKey::Seven, AsciiKeyMap::Lookup('\'')->key);
}

TEST(AsciiKeyMap, ColonIsUnshiftedAndPlusIsShiftedSemicolon) {
    //Assert
    EXPECT_EQ(CompucolorIIKey::Colon, AsciiKeyMap::Lookup(':')->key);
    EXPECT_FALSE(AsciiKeyMap::Lookup(':')->shift);
    EXPECT_EQ(CompucolorIIKey::SemiColon, AsciiKeyMap::Lookup('+')->key);
    EXPECT_TRUE(AsciiKeyMap::Lookup('+')->shift);
}

TEST(AsciiKeyMap, EqualsIsShiftedDash) {
    //Act
    const auto chord = AsciiKeyMap::Lookup('=');

    //Assert
    ASSERT_TRUE(chord.has_value());
    EXPECT_EQ(CompucolorIIKey::Dash, chord->key);
    EXPECT_TRUE(chord->shift);
}

TEST(AsciiKeyMap, NewlineIsCarriageReturn) {
    //Assert
    EXPECT_EQ(CompucolorIIKey::Cr, AsciiKeyMap::Lookup('\n')->key);
    EXPECT_EQ(CompucolorIIKey::Cr, AsciiKeyMap::Lookup('\r')->key);
}

TEST(AsciiKeyMap, UnsupportedCharactersHaveNoMapping) {
    //Assert
    EXPECT_FALSE(AsciiKeyMap::Lookup('{').has_value());
    EXPECT_FALSE(AsciiKeyMap::Lookup('~').has_value());
    EXPECT_FALSE(AsciiKeyMap::Lookup('\x01').has_value());
}
