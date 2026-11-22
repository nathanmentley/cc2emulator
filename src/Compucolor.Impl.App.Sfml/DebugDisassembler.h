#pragma once

#include <cstdint>
#include <string>

namespace Compucolor::Impl::App::Sfml
{
    struct DisassembledInstruction {
        std::string text;
        std::uint8_t length = 1;
    };

    // Decodes the opcode at bytes[0]; bytes must point to at least three readable bytes.
    // The 8080 has no PC-relative instructions, so absolute operands are taken from the
    // immediate bytes and the decoded text does not depend on address.
    DisassembledInstruction Disassemble8080(const std::uint8_t* bytes, std::uint16_t address);
}
