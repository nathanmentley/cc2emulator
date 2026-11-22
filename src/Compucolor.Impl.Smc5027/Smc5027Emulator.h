#pragma once

#include <array>
#include <atomic>

#include <Compucolor/Common/IPluginManager.h>
#include <Compucolor/Logger/ILogger.h>
#include <Compucolor/Smc5027/ISmc5027Emulator.h>

namespace Compucolor::Impl::Smc5027 {
    // 16-register SMC5027 CRT controller. Writes to ports 0x06/0x0B control
    // hardware scrolling and ports 0x0C/0x0D set the cursor position. Register
    // values are atomic because the CRT refresh thread reads them while the
    // emulated CPU writes them.
    class Smc5027Emulator: public Compucolor::Smc5027::ISmc5027Emulator {
        public:
            Smc5027Emulator(
                std::shared_ptr<Logger::ILogger> logger
            );

            // Resets the controller before emulation starts.
            virtual void Start() override;

            // Resets the controller when emulation stops.
            virtual void Stop() override;

            // Persists all 16 registers as an unsigned integer array.
            std::unique_ptr<boost::json::value> SaveState() override;

            // Validates the register array before replacing live state.
            void LoadState(std::shared_ptr<boost::json::value>) override;

            // Returns cursor Y for port 0x08 and cursor X for port 0x09. Other
            // ports are logged and return zero; the production bus does not use
            // this path.
            virtual uint8_t Read(uint8_t port) override;

            // 0x06 sets the scroll register, 0x0B advances it by one row with
            // 32-row wraparound, 0x0C sets the cursor X and 0x0D sets cursor Y
            // masked to six bits. Unknown writes are logged and ignored.
            virtual void Write(uint8_t port, uint8_t data) override;

            // Clears all registers to zero.
            virtual void Reset() override;

            // Cursor column from register 0x09.
            virtual uint8_t GetCursorX() override;

            // Cursor row from register 0x08.
            virtual uint8_t GetCursorY() override;

            // First display row, one past the scroll register with 32-row wrap.
            virtual uint8_t FirstDisplayRow() override;

            // Captures the same cursor/scroll values exposed on the bus into a
            // host-safe snapshot.
            [[nodiscard]] Compucolor::Debug::Smc5027State GetDebugState() override;

            static std::shared_ptr<Compucolor::Smc5027::ISmc5027Emulator> Creator(Common::IPluginManager* manager);
        private:
            static constexpr uint8_t REGISTER_COUNT = 16;
            static constexpr uint8_t ROW_COUNT = 32;

            std::shared_ptr<Logger::ILogger> _logger;
            std::array<std::atomic<uint8_t>, REGISTER_COUNT> _registers{};

            // The bus exposes the SMC at 0x60-0x7F; only the low nibble selects
            // a register.
            static uint8_t ConvertPort(uint8_t port);
    };
}
