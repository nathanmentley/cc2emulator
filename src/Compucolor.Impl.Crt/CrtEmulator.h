
#pragma once

#include <chrono>
#include <atomic>
#include <memory>
#include <thread>

#include <Compucolor/Common/IDisplay.h>
#include <Compucolor/Common/IPluginManager.h>

#include <Compucolor/Logger/ILogger.h>
#include <Compucolor/Scheduler/IScheduler.h>
#include <Compucolor/Memory/IMemory.h>
#include <Compucolor/Smc5027/ISmc5027Emulator.h>
#include <Compucolor/Tms5501/ITms5501Emulator.h>
#include <Compucolor/Crt/ICrtEmulator.h>

namespace Compucolor::Impl::Crt
{
    // Renders the 64x32 character screen from video RAM at 0x7000-0x7FFF every
    // 10 ms. Every 16th refresh triggers the TMS5501 external sensor interrupt;
    // alternating refresh phases implement attribute blink.
    class CrtEmulator: public Compucolor::Crt::ICrtEmulator {
        public:
            CrtEmulator(
                std::shared_ptr<Compucolor::Logger::ILogger> logger,
                std::shared_ptr<Compucolor::Common::IDisplay> display,
                std::shared_ptr<Compucolor::Scheduler::IScheduler> scheduler,
                std::shared_ptr<Compucolor::Memory::IMemory> memory,
                std::shared_ptr<Compucolor::Smc5027::ISmc5027Emulator> smc5027emulator,
                std::shared_ptr<Compucolor::Tms5501::ITms5501Emulator> tms5501Emulator
            );

            // Starts the 10 ms refresh task.
            void Start() override;

            // Stops and joins the refresh task.
            void Stop() override;

            // Switches between the embedded standard and lowercase UF6 fonts.
            void SetLowercaseCharset(bool lowercase) override;

            // Persists the blink phase and selected character set.
            std::unique_ptr<boost::json::value> SaveState() override;

            // Validates phase/charset before applying them.
            void LoadState(std::shared_ptr<boost::json::value>) override;

            static std::shared_ptr<ICrtEmulator> Creator(Compucolor::Common::IPluginManager* manager);

        private:
            static constexpr int Columns = 64;
            static constexpr int Rows = 32;
            static constexpr int CharacterWidth = 6;
            static constexpr int CharacterHeight = 8;
            static constexpr int Width = Columns * CharacterWidth;
            static constexpr int Height = Rows * CharacterHeight;

            std::shared_ptr<Compucolor::Logger::ILogger> _logger;
            std::shared_ptr<Compucolor::Common::IDisplay> _display;
            std::shared_ptr<Compucolor::Scheduler::IScheduler> _scheduler;
            std::shared_ptr<Compucolor::Memory::IMemory> _memory;
            std::shared_ptr<Compucolor::Smc5027::ISmc5027Emulator> _smc5027emulator;
            std::shared_ptr<Compucolor::Tms5501::ITms5501Emulator> _tms5501Emulator;
            std::shared_ptr<Compucolor::Scheduler::IScheduledTask> _loop;


            int _phase;
            std::atomic<bool> _lowercaseCharset{false};

            // Reads each glyph/attribute pair, applies SMC5027 scrolling and draws the cursor.
            void RefreshDisplay() const;

            // Draws the cursor as a solid white cell during the blink-on phase.
            void DrawCursor(uint8_t firstRow) const;

            // Draws one 6x8 cell. With plot set, glyphData is a 2x3 pixel bitmap;
            // otherwise bit 7 selects double-height text and bits 0-6 index the font.
            void DrawGlyph(
                Common::Color foreground,
                Common::Color background,
                uint8_t glyphData,
                bool blink,
                bool plot,
                int x,
                int y
            ) const;

            // True on alternating refreshes, used to blink attributes and the cursor.
            [[nodiscard]] bool IsBlinkOn() const;

            static bool IsBitSet(int bit, uint8_t data);

            // Attribute bits 0-2 and 3-5 select one of eight hardware colors.
            static Common::Color GetForegroundColor(uint8_t data);
            static Common::Color GetBackgroundColor(uint8_t data);
            static Common::Color GetColor(uint8_t data);
    };
}
