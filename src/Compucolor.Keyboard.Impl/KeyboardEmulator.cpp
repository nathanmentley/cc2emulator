#include <Compucolor.Keyboard.Impl/KeyboardEmulator.h>

Compucolor::Keyboard::Impl::KeyboardEmulator::KeyboardEmulator(
    std::shared_ptr<ILogger> logger
):
    _logger(logger),
    _kbMatrix(std::vector<uint8_t>(17))
{
}

void Compucolor::Keyboard::Impl::KeyboardEmulator::Start()
{
    _logger->LogTrace(
        "Starting %s",
        "KeyboardEmulator"
    );

    Reset();
}

void Compucolor::Keyboard::Impl::KeyboardEmulator::Stop()
{
    _logger->LogTrace(
        "Stopping %s",
        "KeyboardEmulator"
    );

    Reset();
}

uint8_t Compucolor::Keyboard::Impl::KeyboardEmulator::Read(uint8_t port)
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

void Compucolor::Keyboard::Impl::KeyboardEmulator::Reset()
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

void Compucolor::Keyboard::Impl::KeyboardEmulator::OnKeyUp(CompucolorIIKey key)
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

void Compucolor::Keyboard::Impl::KeyboardEmulator::OnKeyDown(CompucolorIIKey key)
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

std::optional<int> Compucolor::Keyboard::Impl::KeyboardEmulator::GetRow(CompucolorIIKey key)
{
    switch (key)
    {
        case CompucolorIIKey::Zero: return 15;
        case CompucolorIIKey::One: return 14;
        case CompucolorIIKey::Two: return 13;
        case CompucolorIIKey::Three: return 12;
        case CompucolorIIKey::Four: return 11;
        case CompucolorIIKey::Five: return 10;
        case CompucolorIIKey::Six: return 9;
        case CompucolorIIKey::Seven: return 8;
        case CompucolorIIKey::Eight: return 7;
        case CompucolorIIKey::Nine: return 6;
        case CompucolorIIKey::A: return 14;
        case CompucolorIIKey::B: return 13;
        case CompucolorIIKey::C: return 12;
        case CompucolorIIKey::D: return 11;
        case CompucolorIIKey::E: return 10;
        case CompucolorIIKey::F: return 9;
        case CompucolorIIKey::G: return 8;
        case CompucolorIIKey::H: return 7;
        case CompucolorIIKey::I: return 6;
        case CompucolorIIKey::J: return 5;
        case CompucolorIIKey::K: return 4;
        case CompucolorIIKey::L: return 3;
        case CompucolorIIKey::M: return 2;
        case CompucolorIIKey::N: return 1;
        case CompucolorIIKey::O: return 0;
        case CompucolorIIKey::P: return 15;
        case CompucolorIIKey::Q: return 14;
        case CompucolorIIKey::R: return 13;
        case CompucolorIIKey::S: return 12;
        case CompucolorIIKey::T: return 11;
        case CompucolorIIKey::U: return 10;
        case CompucolorIIKey::V: return 9;
        case CompucolorIIKey::W: return 8;
        case CompucolorIIKey::X: return 7;
        case CompucolorIIKey::Y: return 6;
        case CompucolorIIKey::Z: return 5;
        case CompucolorIIKey::Break: return 15;
        default: return {};
    }
}

 std::optional<int> Compucolor::Keyboard::Impl::KeyboardEmulator::GetBit(CompucolorIIKey key)
{
    switch (key)
    {
        case CompucolorIIKey::Zero: return 0;
        case CompucolorIIKey::One: return 0;
        case CompucolorIIKey::Two: return 0;
        case CompucolorIIKey::Three: return 0;
        case CompucolorIIKey::Four: return 0;
        case CompucolorIIKey::Five: return 0;
        case CompucolorIIKey::Six: return 0;
        case CompucolorIIKey::Seven: return 0;
        case CompucolorIIKey::Eight: return 0;
        case CompucolorIIKey::Nine: return 0;
        case CompucolorIIKey::A: return 1;
        case CompucolorIIKey::B: return 1;
        case CompucolorIIKey::C: return 1;
        case CompucolorIIKey::D: return 1;
        case CompucolorIIKey::E: return 1;
        case CompucolorIIKey::F: return 1;
        case CompucolorIIKey::G: return 1;
        case CompucolorIIKey::H: return 1;
        case CompucolorIIKey::I: return 1;
        case CompucolorIIKey::J: return 1;
        case CompucolorIIKey::K: return 1;
        case CompucolorIIKey::L: return 1;
        case CompucolorIIKey::M: return 1;
        case CompucolorIIKey::N: return 1;
        case CompucolorIIKey::O: return 1;
        case CompucolorIIKey::P: return 2;
        case CompucolorIIKey::Q: return 2;
        case CompucolorIIKey::R: return 2;
        case CompucolorIIKey::S: return 2;
        case CompucolorIIKey::T: return 2;
        case CompucolorIIKey::U: return 2;
        case CompucolorIIKey::V: return 2;
        case CompucolorIIKey::W: return 2;
        case CompucolorIIKey::X: return 2;
        case CompucolorIIKey::Y: return 2;
        case CompucolorIIKey::Z: return 2;
        case CompucolorIIKey::Break: return 4;
        default: return {};
    }
}