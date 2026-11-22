#include "DebugDisassembler.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>

using Compucolor::Impl::App::Sfml::DisassembledInstruction;

namespace
{
    constexpr std::string_view Registers = "BCDEHLMA";
    constexpr std::array<std::string_view, 4> RegisterPairs = {"B", "D", "H", "SP"};
    constexpr std::array<std::string_view, 4> StackPairs = {"B", "D", "H", "PSW"};
    constexpr std::array<std::string_view, 8> Conditions = {"NZ", "Z", "NC", "C", "PO", "PE", "P", "M"};
    constexpr std::array<std::string_view, 8> AluOperations = {"ADD", "ADC", "SUB", "SBB", "ANA", "XRA", "ORA", "CMP"};
    constexpr std::array<std::string_view, 8> ImmediateAluOperations = {"ADI", "ACI", "SUI", "SBI", "ANI", "XRI", "ORI", "CPI"};

    std::string Hex8(std::uint8_t value)
    {
        char buffer[8];
        std::snprintf(buffer, sizeof(buffer), "$%02X", value);
        return std::string(buffer);
    }

    std::string Hex16(std::uint16_t value)
    {
        char buffer[8];
        std::snprintf(buffer, sizeof(buffer), "$%04X", value);
        return std::string(buffer);
    }

    std::string_view Register(std::uint8_t opcode)
    {
        return Registers.substr((opcode >> 3) & 7, 1);
    }

    std::size_t PairIndex(std::uint8_t opcode)
    {
        return (opcode >> 4) & 3;
    }

    std::string_view Condition(std::uint8_t opcode)
    {
        return Conditions[(opcode >> 3) & 7];
    }

    DisassembledInstruction Make(std::string text, std::uint8_t length)
    {
        return {std::move(text), length};
    }
}

DisassembledInstruction Compucolor::Impl::App::Sfml::Disassemble8080(const std::uint8_t* bytes, std::uint16_t address)
{
    // Every control transfer on the 8080 uses an absolute immediate target, so
    // the PC does not affect the output; the parameter is kept for the debug UI.
    (void)address;

    const std::uint8_t opcode = bytes[0];
    const std::uint16_t target = static_cast<std::uint16_t>(bytes[1])
        | static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[2]) << 8);

    if (opcode >= 0x40 && opcode <= 0x7F)
    {
        if (opcode == 0x76)
        {
            return Make("HLT", 1);
        }

        return Make("MOV " + std::string(Register(opcode)) + "," + std::string(1, Registers[opcode & 7]), 1);
    }

    if (opcode >= 0x80 && opcode <= 0xBF)
    {
        return Make(std::string(AluOperations[(opcode >> 3) & 7]) + " " + std::string(1, Registers[opcode & 7]), 1);
    }

    switch (opcode)
    {
        // The 8x opcodes are documented NOPs on the 8080.
        case 0x00: case 0x08: case 0x10: case 0x18:
        case 0x20: case 0x28: case 0x30: case 0x38:
            return Make("NOP", 1);

        case 0x01: case 0x11: case 0x21: case 0x31:
            return Make("LXI " + std::string(RegisterPairs[PairIndex(opcode)]) + "," + Hex16(target), 3);
        case 0x02: case 0x12:
            return Make("STAX " + std::string(RegisterPairs[PairIndex(opcode)]), 1);
        case 0x03: case 0x13: case 0x23: case 0x33:
            return Make("INX " + std::string(RegisterPairs[PairIndex(opcode)]), 1);
        case 0x04: case 0x0C: case 0x14: case 0x1C:
        case 0x24: case 0x2C: case 0x34: case 0x3C:
            return Make("INR " + std::string(Register(opcode)), 1);
        case 0x05: case 0x0D: case 0x15: case 0x1D:
        case 0x25: case 0x2D: case 0x35: case 0x3D:
            return Make("DCR " + std::string(Register(opcode)), 1);
        case 0x06: case 0x0E: case 0x16: case 0x1E:
        case 0x26: case 0x2E: case 0x36: case 0x3E:
            return Make("MVI " + std::string(Register(opcode)) + "," + Hex8(bytes[1]), 2);
        case 0x07:
            return Make("RLC", 1);
        case 0x09: case 0x19: case 0x29: case 0x39:
            return Make("DAD " + std::string(RegisterPairs[PairIndex(opcode)]), 1);
        case 0x0A: case 0x1A:
            return Make("LDAX " + std::string(RegisterPairs[PairIndex(opcode)]), 1);
        case 0x0B: case 0x1B: case 0x2B: case 0x3B:
            return Make("DCX " + std::string(RegisterPairs[PairIndex(opcode)]), 1);
        case 0x0F:
            return Make("RRC", 1);
        case 0x17:
            return Make("RAL", 1);
        case 0x1F:
            return Make("RAR", 1);
        case 0x22:
            return Make("SHLD " + Hex16(target), 3);
        case 0x27:
            return Make("DAA", 1);
        case 0x2A:
            return Make("LHLD " + Hex16(target), 3);
        case 0x2F:
            return Make("CMA", 1);
        case 0x32:
            return Make("STA " + Hex16(target), 3);
        case 0x37:
            return Make("STC", 1);
        case 0x3A:
            return Make("LDA " + Hex16(target), 3);
        case 0x3F:
            return Make("CMC", 1);

        case 0xC0: case 0xC8: case 0xD0: case 0xD8:
        case 0xE0: case 0xE8: case 0xF0: case 0xF8:
            return Make("R" + std::string(Condition(opcode)), 1);
        case 0xC1: case 0xD1: case 0xE1: case 0xF1:
            return Make("POP " + std::string(StackPairs[PairIndex(opcode)]), 1);
        case 0xC2: case 0xCA: case 0xD2: case 0xDA:
        case 0xE2: case 0xEA: case 0xF2: case 0xFA:
            return Make("J" + std::string(Condition(opcode)) + " " + Hex16(target), 3);
        case 0xC3: case 0xCB:
            return Make("JMP " + Hex16(target), 3);
        case 0xC4: case 0xCC: case 0xD4: case 0xDC:
        case 0xE4: case 0xEC: case 0xF4: case 0xFC:
            return Make("C" + std::string(Condition(opcode)) + " " + Hex16(target), 3);
        case 0xC5: case 0xD5: case 0xE5: case 0xF5:
            return Make("PUSH " + std::string(StackPairs[PairIndex(opcode)]), 1);
        case 0xC6: case 0xCE: case 0xD6: case 0xDE:
        case 0xE6: case 0xEE: case 0xF6: case 0xFE:
            return Make(std::string(ImmediateAluOperations[(opcode >> 3) & 7]) + " " + Hex8(bytes[1]), 2);
        case 0xC7: case 0xCF: case 0xD7: case 0xDF:
        case 0xE7: case 0xEF: case 0xF7: case 0xFF:
            return Make("RST " + std::to_string((opcode >> 3) & 7), 1);
        case 0xC9: case 0xD9:
            return Make("RET", 1);
        case 0xCD: case 0xDD: case 0xED: case 0xFD:
            return Make("CALL " + Hex16(target), 3);
        case 0xD3:
            return Make("OUT " + Hex8(bytes[1]), 2);
        case 0xDB:
            return Make("IN " + Hex8(bytes[1]), 2);
        case 0xE3:
            return Make("XTHL", 1);
        case 0xE9:
            return Make("PCHL", 1);
        case 0xEB:
            return Make("XCHG", 1);
        case 0xF3:
            return Make("DI", 1);
        case 0xF9:
            return Make("SPHL", 1);
        case 0xFB:
            return Make("EI", 1);
    }

    // Unreachable: every opcode 0x00..0xFF is handled above. Keep a visible
    // marker so a missing case fails loudly in the debug UI and in tests.
    return Make("???", 1);
}
