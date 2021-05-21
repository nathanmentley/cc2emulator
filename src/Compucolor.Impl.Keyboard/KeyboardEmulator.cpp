#include "KeyboardEmulator.h"

Compucolor::Impl::Keyboard::KeyboardEmulator::KeyboardEmulator(
    std::shared_ptr<Logger::ILogger> logger
):
    _logger(logger),
    _kbMatrix(std::vector<uint8_t>(17))
{
}

void Compucolor::Impl::Keyboard::KeyboardEmulator::Start()
{
    _logger->LogTrace(
        "Starting %s",
        "KeyboardEmulator"
    );

    Reset();
}

void Compucolor::Impl::Keyboard::KeyboardEmulator::Stop()
{
    _logger->LogTrace(
        "Stopping %s",
        "KeyboardEmulator"
    );

    Reset();
}

uint8_t Compucolor::Impl::Keyboard::KeyboardEmulator::Read(uint8_t port)
{
    /*
    _logger->LogTrace(
        "Reading keyboard byte %d in %s",
        port,
        "KeyboardEmulator"
    );
    */

    return _kbMatrix[port];
}

void Compucolor::Impl::Keyboard::KeyboardEmulator::Reset()
{
    _logger->LogTrace(
        "Resetting in %s",
        "KeyboardEmulator"
    );

    for(uint8_t i = 0; i < 17; i++)
    {
        _kbMatrix[i] = 0xFF;
    }
}

void Compucolor::Impl::Keyboard::KeyboardEmulator::OnKeyUp(Compucolor::Keyboard::CompucolorIIKey key)
{
    std::optional<int> row = GetRow(key);
    std::optional<int> bit = GetBit(key);

    _logger->LogTrace(
        "OnKeyUp for %d (%d, %d) in %s",
        key,
        row,
        bit,
        "KeyboardEmulator"
    );

    if(row.has_value() && bit.has_value())
    {
        _kbMatrix[row.value()] = 0xFF;
    }
}

void Compucolor::Impl::Keyboard::KeyboardEmulator::OnKeyDown(Compucolor::Keyboard::CompucolorIIKey key)
{
    std::optional<int> row = GetRow(key);
    std::optional<int> bit = GetBit(key);

    _logger->LogTrace(
        "OnKeyDown for %d (%d, %d) in %s",
        key,
        row,
        bit,
        "KeyboardEmulator"
    );

    if(row.has_value() && bit.has_value())
    {
        _kbMatrix[row.value()]  &= (~(1 << bit.value()) & 0xFF);
    }
}

std::optional<int> Compucolor::Impl::Keyboard::KeyboardEmulator::GetRow(Compucolor::Keyboard::CompucolorIIKey key)
{
    switch (key)
    {
        case Compucolor::Keyboard::CompucolorIIKey::Zero: return 15;
        case Compucolor::Keyboard::CompucolorIIKey::One: return 14;
        case Compucolor::Keyboard::CompucolorIIKey::Two: return 13;
        case Compucolor::Keyboard::CompucolorIIKey::Three: return 12;
        case Compucolor::Keyboard::CompucolorIIKey::Four: return 11;
        case Compucolor::Keyboard::CompucolorIIKey::Five: return 10;
        case Compucolor::Keyboard::CompucolorIIKey::Six: return 9;
        case Compucolor::Keyboard::CompucolorIIKey::Seven: return 8;
        case Compucolor::Keyboard::CompucolorIIKey::Eight: return 7;
        case Compucolor::Keyboard::CompucolorIIKey::Nine: return 6;
        case Compucolor::Keyboard::CompucolorIIKey::A: return 14;
        case Compucolor::Keyboard::CompucolorIIKey::B: return 13;
        case Compucolor::Keyboard::CompucolorIIKey::C: return 12;
        case Compucolor::Keyboard::CompucolorIIKey::D: return 11;
        case Compucolor::Keyboard::CompucolorIIKey::E: return 10;
        case Compucolor::Keyboard::CompucolorIIKey::F: return 9;
        case Compucolor::Keyboard::CompucolorIIKey::G: return 8;
        case Compucolor::Keyboard::CompucolorIIKey::H: return 7;
        case Compucolor::Keyboard::CompucolorIIKey::I: return 6;
        case Compucolor::Keyboard::CompucolorIIKey::J: return 5;
        case Compucolor::Keyboard::CompucolorIIKey::K: return 4;
        case Compucolor::Keyboard::CompucolorIIKey::L: return 3;
        case Compucolor::Keyboard::CompucolorIIKey::M: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::N: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::O: return 0;
        case Compucolor::Keyboard::CompucolorIIKey::P: return 15;
        case Compucolor::Keyboard::CompucolorIIKey::Q: return 14;
        case Compucolor::Keyboard::CompucolorIIKey::R: return 13;
        case Compucolor::Keyboard::CompucolorIIKey::S: return 12;
        case Compucolor::Keyboard::CompucolorIIKey::T: return 11;
        case Compucolor::Keyboard::CompucolorIIKey::U: return 10;
        case Compucolor::Keyboard::CompucolorIIKey::V: return 9;
        case Compucolor::Keyboard::CompucolorIIKey::W: return 8;
        case Compucolor::Keyboard::CompucolorIIKey::X: return 7;
        case Compucolor::Keyboard::CompucolorIIKey::Y: return 6;
        case Compucolor::Keyboard::CompucolorIIKey::Z: return 5;
        case Compucolor::Keyboard::CompucolorIIKey::Break: return 15;
        default: return {};
    }
}

 std::optional<int> Compucolor::Impl::Keyboard::KeyboardEmulator::GetBit(Compucolor::Keyboard::CompucolorIIKey key)
{
    switch (key)
    {
        case Compucolor::Keyboard::CompucolorIIKey::Zero: return 0;
        case Compucolor::Keyboard::CompucolorIIKey::One: return 0;
        case Compucolor::Keyboard::CompucolorIIKey::Two: return 0;
        case Compucolor::Keyboard::CompucolorIIKey::Three: return 0;
        case Compucolor::Keyboard::CompucolorIIKey::Four: return 0;
        case Compucolor::Keyboard::CompucolorIIKey::Five: return 0;
        case Compucolor::Keyboard::CompucolorIIKey::Six: return 0;
        case Compucolor::Keyboard::CompucolorIIKey::Seven: return 0;
        case Compucolor::Keyboard::CompucolorIIKey::Eight: return 0;
        case Compucolor::Keyboard::CompucolorIIKey::Nine: return 0;
        case Compucolor::Keyboard::CompucolorIIKey::A: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::B: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::C: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::D: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::E: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::F: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::G: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::H: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::I: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::J: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::K: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::L: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::M: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::N: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::O: return 1;
        case Compucolor::Keyboard::CompucolorIIKey::P: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::Q: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::R: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::S: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::T: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::U: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::V: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::W: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::X: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::Y: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::Z: return 2;
        case Compucolor::Keyboard::CompucolorIIKey::Break: return 4;
        default: return {};
    }
}