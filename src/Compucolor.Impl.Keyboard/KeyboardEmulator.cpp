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

std::unique_ptr<boost::json::value> Compucolor::Impl::Keyboard::KeyboardEmulator::SaveState()
{
    std::lock_guard lock(_mutex);

    boost::json::object state;
    boost::json::array matrix;

    matrix.reserve(_kbMatrix.size());

    for (const auto& row : _kbMatrix)
    {
        matrix.push_back(
            static_cast<uint64_t>(
                row.load(std::memory_order_relaxed)
            )
        );
    }

    state["matrix"] = std::move(matrix);

    return std::make_unique<boost::json::value>(
        std::move(state)
    );
}

void Compucolor::Impl::Keyboard::KeyboardEmulator::LoadState(
    std::shared_ptr<boost::json::value> value
)
{
    std::lock_guard lock(_mutex);

    if (!value || !value->is_object())
    {
        throw std::runtime_error("Invalid KeyboardEmulator state");
    }

    const auto& state = value->as_object();
    const auto& matrixJson = state.at("matrix").as_array();

    if (matrixJson.size() != _kbMatrix.size())
    {
        throw std::runtime_error("Invalid keyboard matrix size");
    }

    std::array<uint8_t, 17> matrix{};

    for (std::size_t i = 0; i < matrixJson.size(); i++)
    {
        const auto& entry = matrixJson[i];

        uint64_t byte;

        if (entry.is_uint64())
        {
            byte = entry.as_uint64();
        }
        else if (entry.is_int64() && entry.as_int64() >= 0)
        {
            byte = static_cast<uint64_t>(entry.as_int64());
        }
        else
        {
            throw std::runtime_error(
                "Invalid value in keyboard matrix"
            );
        }

        if (byte > 255)
        {
            throw std::runtime_error(
                "Keyboard matrix value exceeds 255"
            );
        }

        matrix[i] = static_cast<uint8_t>(byte);
    }

    // Restore the matrix only after the entire state has been validated.
    for (std::size_t i = 0; i < matrix.size(); i++)
    {
        _kbMatrix[i].store(
            matrix[i],
            std::memory_order_relaxed
        );
    }

    // Transient scan state belongs to the host keyboard, not the saved machine.
    for (auto& row : _pressCounts)
    {
        row.fill(0);
    }

    for (auto& row : _pendingReleaseScans)
    {
        row.fill(0);
    }

    for (auto& row : _observedScans)
    {
        row.fill(0);
    }
}

uint8_t Compucolor::Impl::Keyboard::KeyboardEmulator::Read(uint8_t port)
{
    if (port >= _kbMatrix.size())
    {
        return 0xFF;
    }

    std::lock_guard lock(_mutex);

    // Return the state that includes a pending release first, then open that
    // contact for the next scan. This makes even a zero-duration host tap
    // visible to the CPU exactly once.
    const uint8_t value = _kbMatrix[port].load(std::memory_order_relaxed);

    auto& observedRow = _observedScans[port];
    auto& pendingRow = _pendingReleaseScans[port];

    for (int bit = 0; bit < 8; ++bit)
    {
        const uint8_t mask = static_cast<uint8_t>(1U << bit);

        if ((value & mask) == 0 && observedRow[bit] < DebounceScans)
        {
            ++observedRow[bit];
        }

        if (pendingRow[bit] == 0)
        {
            continue;
        }

        --pendingRow[bit];

        if (pendingRow[bit] == 0)
        {
            _kbMatrix[port].fetch_or(
                static_cast<uint8_t>(1U << bit),
                std::memory_order_relaxed
            );
        }
    }

    return value;
}

void Compucolor::Impl::Keyboard::KeyboardEmulator::Reset()
{
    _logger->LogTrace(
        "Resetting in %s",
        "KeyboardEmulator"
    );

    std::lock_guard lock(_mutex);

    for(uint8_t i = 0; i < 17; i++)
    {
        _kbMatrix[i].store(0xFF, std::memory_order_relaxed);
    }

    for (auto& row : _pressCounts)
    {
        row.fill(0);
    }

    for (auto& row : _pendingReleaseScans)
    {
        row.fill(0);
    }

    for (auto& row : _observedScans)
    {
        row.fill(0);
    }
}

void Compucolor::Impl::Keyboard::KeyboardEmulator::OnKeyUp(Compucolor::Keyboard::CompucolorIIKey key)
{
    const std::optional<int> row = GetRow(key);
    const std::optional<int> bit = GetBit(key);

    _logger->LogTrace(
        "OnKeyUp for %d (%d, %d) in %s",
        static_cast<int>(key),
        row.value_or(-1),
        bit.value_or(-1),
        "KeyboardEmulator"
    );

    if (!row.has_value() || !bit.has_value())
    {
        return;
    }

    std::lock_guard lock(_mutex);

    const uint8_t mask = static_cast<uint8_t>(1U << bit.value());
    uint8_t& count = _pressCounts[row.value()][bit.value()];

    if (count > 0)
    {
        --count;
    }

    if (count > 0)
    {
        return;
    }

    // Row 16 holds modifiers. The ROM only samples that row while another key
    // is pressed, so latching a modifier release would leave Shift/Ctrl stuck
    // until the next key press and turn that key into its shifted code.
    // Modifiers must therefore always be released immediately.
    if (row.value() == 16)
    {
        _kbMatrix[row.value()].fetch_or(mask, std::memory_order_relaxed);
        _observedScans[row.value()][bit.value()] = 0;
        _pendingReleaseScans[row.value()][bit.value()] = 0;
        return;
    }

    uint8_t& observed = _observedScans[row.value()][bit.value()];

    if (observed >= DebounceScans)
    {
        // The CPU already saw this contact while it was pressed.
        _kbMatrix[row.value()].fetch_or(mask, std::memory_order_relaxed);
        observed = 0;
        _pendingReleaseScans[row.value()][bit.value()] = 0;
        return;
    }

    // Keep the contact closed until the CPU has had one chance to scan it.
    _pendingReleaseScans[row.value()][bit.value()] = DebounceScans - observed;
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

    if (!row.has_value() || !bit.has_value())
    {
        return;
    }

    std::lock_guard lock(_mutex);

    const uint8_t mask = static_cast<uint8_t>(1U << bit.value());
    uint8_t& count = _pressCounts[row.value()][bit.value()];

    if (count == 0)
    {
        // A new press supersedes any release waiting for a scan.
        _pendingReleaseScans[row.value()][bit.value()] = 0;
        _observedScans[row.value()][bit.value()] = 0;
    }

    ++count;
    _kbMatrix[row.value()].fetch_and(
        static_cast<uint8_t>(~mask),
        std::memory_order_relaxed
    );
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
