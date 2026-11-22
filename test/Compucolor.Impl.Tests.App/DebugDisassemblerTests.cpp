#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <initializer_list>
#include <string>

#include <gtest/gtest.h>

#include "DebugDisassembler.h"

using Compucolor::Impl::App::Sfml::DisassembledInstruction;
using Compucolor::Impl::App::Sfml::Disassemble8080;

namespace
{
    DisassembledInstruction Decode(std::initializer_list<std::uint8_t> bytes, std::uint16_t address = 0x0100)
    {
        std::array<std::uint8_t, 3> buffer{};
        std::size_t index = 0;
        for (const std::uint8_t byte : bytes)
        {
            buffer[index++] = byte;
        }

        return Disassemble8080(buffer.data(), address);
    }
}

TEST(DebugDisassembler, DecodesRepresentativeOneByteInstructions) {
    struct TestCase {
        std::uint8_t opcode;
        const char* text;
    };

    constexpr std::array<TestCase, 18> cases{{
        {0x00, "NOP"}, {0x38, "NOP"}, {0x07, "RLC"}, {0x0F, "RRC"},
        {0x17, "RAL"}, {0x1F, "RAR"}, {0x27, "DAA"}, {0x2F, "CMA"},
        {0x37, "STC"}, {0x3F, "CMC"}, {0x76, "HLT"}, {0xC0, "RNZ"},
        {0xE8, "RPE"}, {0xC9, "RET"}, {0xD9, "RET"}, {0xE3, "XTHL"},
        {0xEB, "XCHG"}, {0xFF, "RST 7"}
    }};

    for (const auto& test : cases)
    {
        const auto result = Decode({test.opcode});
        EXPECT_EQ(test.text, result.text) << "opcode " << std::hex << static_cast<int>(test.opcode);
        EXPECT_EQ(1, result.length) << "opcode " << std::hex << static_cast<int>(test.opcode);
    }
}

TEST(DebugDisassembler, DecodesEveryMovInstruction) {
    constexpr std::array<char, 8> registers = {'B', 'C', 'D', 'E', 'H', 'L', 'M', 'A'};

    for (int destination = 0; destination < 8; destination++)
    {
        for (int source = 0; source < 8; source++)
        {
            const auto opcode = static_cast<std::uint8_t>(0x40 | (destination << 3) | source);
            const auto result = Decode({opcode});

            if (opcode == 0x76)
            {
                EXPECT_EQ("HLT", result.text);
                EXPECT_EQ(1, result.length);
                continue;
            }

            const std::string expected = std::string("MOV ") + registers[destination] + "," + registers[source];
            EXPECT_EQ(expected, result.text) << "opcode " << std::hex << static_cast<int>(opcode);
            EXPECT_EQ(1, result.length) << "opcode " << std::hex << static_cast<int>(opcode);
        }
    }
}

TEST(DebugDisassembler, DecodesEveryRegisterAluInstruction) {
    constexpr std::array<const char*, 8> operations = {
        "ADD", "ADC", "SUB", "SBB", "ANA", "XRA", "ORA", "CMP"
    };
    constexpr std::array<char, 8> registers = {'B', 'C', 'D', 'E', 'H', 'L', 'M', 'A'};

    for (int operation = 0; operation < 8; operation++)
    {
        for (int source = 0; source < 8; source++)
        {
            const auto opcode = static_cast<std::uint8_t>(0x80 | (operation << 3) | source);
            const std::string expected = std::string(operations[operation]) + " " + registers[source];
            const auto result = Decode({opcode});

            EXPECT_EQ(expected, result.text) << "opcode " << std::hex << static_cast<int>(opcode);
            EXPECT_EQ(1, result.length) << "opcode " << std::hex << static_cast<int>(opcode);
        }
    }
}

TEST(DebugDisassembler, DecodesTwoByteInstructions) {
    struct TestCase {
        std::uint8_t opcode;
        std::uint8_t operand;
        const char* text;
    };

    constexpr std::array<TestCase, 9> cases{{
        {0x06, 0x42, "MVI B,$42"}, {0x3E, 0x42, "MVI A,$42"},
        {0x36, 0xFF, "MVI M,$FF"}, {0xC6, 0x10, "ADI $10"},
        {0xDE, 0x01, "SBI $01"}, {0xEE, 0x80, "XRI $80"},
        {0xFE, 0x05, "CPI $05"}, {0xD3, 0x10, "OUT $10"},
        {0xDB, 0xFF, "IN $FF"}
    }};

    for (const auto& test : cases)
    {
        const auto result = Decode({test.opcode, test.operand});
        EXPECT_EQ(test.text, result.text) << "opcode " << std::hex << static_cast<int>(test.opcode);
        EXPECT_EQ(2, result.length) << "opcode " << std::hex << static_cast<int>(test.opcode);
    }
}

TEST(DebugDisassembler, DecodesThreeByteInstructions) {
    struct TestCase {
        std::uint8_t opcode;
        const char* text;
    };

    // Every operand is $1234 in little-endian order.
    constexpr std::array<TestCase, 18> cases{{
        {0x01, "LXI B,$1234"}, {0x21, "LXI H,$1234"},
        {0x22, "SHLD $1234"}, {0x2A, "LHLD $1234"},
        {0x32, "STA $1234"}, {0x3A, "LDA $1234"},
        {0xC2, "JNZ $1234"}, {0xDA, "JC $1234"}, {0xEA, "JPE $1234"},
        {0xC3, "JMP $1234"}, {0xCB, "JMP $1234"},
        {0xC4, "CNZ $1234"}, {0xDC, "CC $1234"}, {0xE4, "CPO $1234"},
        {0xFC, "CM $1234"}, {0xCD, "CALL $1234"}, {0xDD, "CALL $1234"},
        {0xED, "CALL $1234"}
    }};

    for (const auto& test : cases)
    {
        const auto result = Decode({test.opcode, 0x34, 0x12});
        EXPECT_EQ(test.text, result.text) << "opcode " << std::hex << static_cast<int>(test.opcode);
        EXPECT_EQ(3, result.length) << "opcode " << std::hex << static_cast<int>(test.opcode);
    }

    const auto lastAlias = Decode({0xFD, 0x34, 0x12});
    EXPECT_EQ("CALL $1234", lastAlias.text);
    EXPECT_EQ(3, lastAlias.length);
}

TEST(DebugDisassembler, TextDoesNotDependOnProgramCounter) {
    constexpr std::array<std::uint8_t, 3> bytes{0xC3, 0x00, 0x20};

    const auto atZero = Disassemble8080(bytes.data(), 0x0000);
    const auto atEnd = Disassemble8080(bytes.data(), 0xFFFE);

    EXPECT_EQ("JMP $2000", atZero.text);
    EXPECT_EQ(atZero.text, atEnd.text);
    EXPECT_EQ(atZero.length, atEnd.length);
}

TEST(DebugDisassembler, EveryOpcodeDecodesToAKnownInstruction) {
    for (int opcode = 0; opcode <= 0xFF; opcode++)
    {
        const std::array<std::uint8_t, 3> bytes{
            static_cast<std::uint8_t>(opcode), 0x34, 0x12
        };
        const auto result = Disassemble8080(bytes.data(), 0x1000);

        EXPECT_FALSE(result.text.empty()) << "opcode " << std::hex << opcode;
        EXPECT_NE("???", result.text) << "opcode " << std::hex << opcode;
        EXPECT_TRUE(result.length == 1 || result.length == 2 || result.length == 3)
            << "opcode " << std::hex << opcode;
    }
}
