
#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>

#include <Compucolor/Common/IPluginManager.h>
#include <Compucolor/Keyboard/IKeyboardEmulator.h>
#include <Compucolor/Logger/ILogger.h>

namespace Compucolor::Impl::Keyboard
{
    // 17x8 active-low keyboard matrix read by the TMS5501 on port 1.
    // Save/load state persists the matrix but not transient debounce counters.
    class KeyboardEmulator: public Compucolor::Keyboard::IKeyboardEmulator {
        public:
            KeyboardEmulator(
                std::shared_ptr<Logger::ILogger> logger
            );

            void Start() override;
            void Stop() override;

            std::unique_ptr<boost::json::value> SaveState() override;
            void LoadState(std::shared_ptr<boost::json::value>) override;

            // Returns the row selected by port & 0x0F, or 0xFF for unknown rows.
            // One-scan pending releases keep very short host taps visible to the ROM.
            uint8_t Read(uint8_t port) override;

            // Clears the matrix to all-ones (no keys pressed).
            void Reset() override;

            // Releases a matrix contact, reference-counted across alias keys.
            void OnKeyUp(Compucolor::Keyboard::CompucolorIIKey key) override;

            // Presses a matrix contact; modifiers in row 16 release immediately.
            void OnKeyDown(Compucolor::Keyboard::CompucolorIIKey key) override;

            static std::shared_ptr<Compucolor::Keyboard::IKeyboardEmulator> Creator(Common::IPluginManager* manager);

        private:
            std::shared_ptr<Logger::ILogger> _logger;
            std::array<std::atomic<uint8_t>, 17> _kbMatrix{};

            // A host tap can begin and end between two keyboard scans. Keep
            // such a press closed until the next scan so it is not lost.
            // Row 16 is exempt because the ROM only samples modifiers while
            // another key is pressed. Counts are per matrix contact because
            // aliases share contacts.
            static constexpr uint8_t DebounceScans = 1;

            mutable std::mutex _mutex;
            std::array<std::array<uint8_t, 8>, 17> _pressCounts{};
            std::array<std::array<uint8_t, 8>, 17> _pendingReleaseScans{};
            std::array<std::array<uint8_t, 8>, 17> _observedScans{};

            static std::optional<int> GetRow(Compucolor::Keyboard::CompucolorIIKey key);

            static std::optional<int> GetBit(Compucolor::Keyboard::CompucolorIIKey key);
    };
}
