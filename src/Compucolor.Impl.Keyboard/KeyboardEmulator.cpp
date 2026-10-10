#include "KeyboardEmulator.h"

#include <utility>

Compucolor::Impl::Keyboard::KeyboardEmulator::KeyboardEmulator(
    std::shared_ptr<Logger::ILogger> logger
):
    _logger(std::move(logger))
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

    return _kbMatrix[port].load(std::memory_order_relaxed);
}

void Compucolor::Impl::Keyboard::KeyboardEmulator::Reset()
{
    _logger->LogTrace(
        "Resetting in %s",
        "KeyboardEmulator"
    );

    for(uint8_t i = 0; i < 17; i++)
    {
        _kbMatrix[i].store(0xFF, std::memory_order_relaxed);
    }
}

void Compucolor::Impl::Keyboard::KeyboardEmulator::OnKeyUp(Compucolor::Keyboard::CompucolorIIKey key)
{
    std::optional<int> row = GetRow(key);
    std::optional<int> bit = GetBit(key);

    _logger->LogTrace(
        "OnKeyUp for %d (%d, %d) in %s",
        static_cast<int>(key),
        row.value_or(-1),
        bit.value_or(-1),
        "KeyboardEmulator"
    );

    if(row.has_value() && bit.has_value())
    {
        _kbMatrix[row.value()].fetch_or(
            static_cast<uint8_t>(1 << bit.value()),
            std::memory_order_relaxed
        );
    }
}

void Compucolor::Impl::Keyboard::KeyboardEmulator::OnKeyDown(Compucolor::Keyboard::CompucolorIIKey key)
{
    const std::optional<int> row = GetRow(key);
    const std::optional<int> bit = GetBit(key);

    _logger->LogTrace(
        "OnKeyDown for %d (%d, %d) in %s",
        static_cast<int>(key),
        row.value_or(-1),
        bit.value_or(-1),
        "KeyboardEmulator"
    );

    if (!row.has_value()) {
        //throw std::invalid_argument("received invalid value: " + std::to_string(static_cast<int>(key)));
    }

    if (!bit.has_value()) {
        //throw std::invalid_argument("received invalid value: " + std::to_string(static_cast<int>(key)));
    }

    if(row.has_value() && bit.has_value())
    {
        _kbMatrix[row.value()].fetch_and(
            static_cast<uint8_t>(~(1 << bit.value()) & 0xFF),
            std::memory_order_relaxed
        );
    }
}

namespace
{
    using Compucolor::Keyboard::CompucolorIIKey;

    struct MatrixPosition
    {
        int row;
        int bit;
    };

    // Rows and bits follow the Compucolor II keyboard matrix as documented by ccemu.
    std::optional<MatrixPosition> FindPosition(const CompucolorIIKey key)
    {
        const auto index = static_cast<int>(key);

        const auto offset = [index](const CompucolorIIKey first) { return index - static_cast<int>(first); };

        // Digits take rows 15..6 on bit 0; the punctuation keys below them follow.
        if (key >= CompucolorIIKey::Zero && key <= CompucolorIIKey::Nine)
            return MatrixPosition{15 - offset(CompucolorIIKey::Zero), 0};

        if (key >= CompucolorIIKey::Comma && key <= CompucolorIIKey::Slash)
            return MatrixPosition{3 - offset(CompucolorIIKey::Comma), 0};

        // AtSign..O share bit 1.
        if (key >= CompucolorIIKey::AtSign && key <= CompucolorIIKey::O)
            return MatrixPosition{15 - offset(CompucolorIIKey::AtSign), 1};

        // P..Underscore share bit 2.
        if (key >= CompucolorIIKey::P && key <= CompucolorIIKey::Underscore)
            return MatrixPosition{15 - offset(CompucolorIIKey::P), 2};

        if (key >= CompucolorIIKey::F0 && key <= CompucolorIIKey::F15)
            return MatrixPosition{15 - offset(CompucolorIIKey::F0), 3};

        if (key >= CompucolorIIKey::Num0 && key <= CompucolorIIKey::Num9)
            return MatrixPosition{15 - offset(CompucolorIIKey::Num0), 0};

        switch (key)
        {
            case CompucolorIIKey::Colon: return MatrixPosition{5, 0};
            case CompucolorIIKey::SemiColon: return MatrixPosition{4, 0};

            case CompucolorIIKey::Break: return MatrixPosition{15, 4};
            case CompucolorIIKey::Inschar: return MatrixPosition{14, 4};
            case CompucolorIIKey::Delline: return MatrixPosition{13, 4};
            case CompucolorIIKey::Insline: return MatrixPosition{12, 4};
            case CompucolorIIKey::Delchar: return MatrixPosition{11, 4};
            case CompucolorIIKey::Auto: return MatrixPosition{10, 4};
            case CompucolorIIKey::Home: return MatrixPosition{7, 4};
            case CompucolorIIKey::Tab: return MatrixPosition{6, 4};
            case CompucolorIIKey::CurDwn: return MatrixPosition{5, 4};
            case CompucolorIIKey::ELine: return MatrixPosition{4, 4};
            case CompucolorIIKey::EPage: return MatrixPosition{3, 4};
            case CompucolorIIKey::Cr: return MatrixPosition{2, 4};
            case CompucolorIIKey::A7on: return MatrixPosition{1, 4};
            case CompucolorIIKey::Bla7off: return MatrixPosition{0, 4};

            case CompucolorIIKey::Black: return MatrixPosition{15, 5};
            case CompucolorIIKey::Red: return MatrixPosition{14, 5};
            case CompucolorIIKey::Green: return MatrixPosition{13, 5};
            case CompucolorIIKey::Yellow: return MatrixPosition{12, 5};
            case CompucolorIIKey::Blue: return MatrixPosition{11, 5};
            case CompucolorIIKey::Magenta: return MatrixPosition{10, 5};
            case CompucolorIIKey::Cyan: return MatrixPosition{9, 5};
            case CompucolorIIKey::White: return MatrixPosition{8, 5};
            case CompucolorIIKey::Currgt: return MatrixPosition{6, 5};
            case CompucolorIIKey::Curlft: return MatrixPosition{5, 5};
            case CompucolorIIKey::Esc: return MatrixPosition{4, 5};
            case CompucolorIIKey::CurUp: return MatrixPosition{3, 5};
            case CompucolorIIKey::FgOn: return MatrixPosition{2, 5};
            case CompucolorIIKey::BgOn: return MatrixPosition{1, 5};
            case CompucolorIIKey::BlinkOn: return MatrixPosition{0, 5};

            case CompucolorIIKey::Space: return MatrixPosition{15, 6};

            // Row 16 holds the modifier keys.
            case CompucolorIIKey::Capslock: return MatrixPosition{16, 7};
            case CompucolorIIKey::Repeat: return MatrixPosition{16, 6};
            case CompucolorIIKey::Shift: return MatrixPosition{16, 5};
            case CompucolorIIKey::Ctrl: return MatrixPosition{16, 4};

            // Aliases of keys that exist on the real keyboard.
            case CompucolorIIKey::Bksp: return MatrixPosition{5, 5};
            case CompucolorIIKey::Delete: return MatrixPosition{11, 4};
            case CompucolorIIKey::Insert: return MatrixPosition{14, 4};
            case CompucolorIIKey::NumDash: return MatrixPosition{2, 0};
            case CompucolorIIKey::NumSlash: return MatrixPosition{0, 0};
            case CompucolorIIKey::NumPeriod: return MatrixPosition{1, 0};

            default: return std::nullopt;
        }
    }
}

std::optional<int> Compucolor::Impl::Keyboard::KeyboardEmulator::GetRow(Compucolor::Keyboard::CompucolorIIKey key)
{
    if (const auto position = FindPosition(key))
        return position->row;

    return {};
}

std::optional<int> Compucolor::Impl::Keyboard::KeyboardEmulator::GetBit(Compucolor::Keyboard::CompucolorIIKey key)
{
    if (const auto position = FindPosition(key))
        return position->bit;

    return {};
}

std::shared_ptr<Compucolor::Keyboard::IKeyboardEmulator> Compucolor::Impl::Keyboard::KeyboardEmulator::Creator(
    Common::IPluginManager* manager
)
{
    return std::make_shared<KeyboardEmulator>(manager->GetLogger());
}
