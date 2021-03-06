
#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <thread>

#include <Compucolor/Common/IDisplay.h>

#include <Compucolor/Logger/ILogger.h>
#include <Compucolor/Scheduler/IScheduler.h>
#include <Compucolor/Memory/IMemory.h>
#include <Compucolor/Smc5027/ISmc5027Emulator.h>
#include <Compucolor/Crt/ICrtEmulator.h>

#include "FontRom.h"

namespace Compucolor::Impl::Crt
{
    /**
     * @brief  
     * @note   
     * @retval None
     */
    class CrtEmulator: public Compucolor::Crt::ICrtEmulator {
        public:
            /**
             * @brief  
             * @note   
             * @param  memory: 
             * @param  smc5027emulator: 
             * @retval 
             */
            CrtEmulator(
                std::shared_ptr<Compucolor::Logger::ILogger> logger,
                std::shared_ptr<Compucolor::Common::IDisplay> display,
                std::shared_ptr<Compucolor::Scheduler::IScheduler> scheduler,
                std::shared_ptr<Compucolor::Memory::IMemory> memory,
                std::shared_ptr<Compucolor::Smc5027::ISmc5027Emulator> smc5027emulator
            );

            /**
             * @brief  
             * @note   
             * @retval None
             */
            virtual void Start() override;

            /**
             * @brief  
             * @note   
             * @retval None
             */
            virtual void Stop() override;

        private:
            static int const Columns = 64;
            static int const Rows = 32;
            static int const CharacterWidth = 6;
            static int const CharacterHeight = 8;
            static int const Width = Columns * CharacterWidth;
            static int const Height = Rows * CharacterHeight;

            std::shared_ptr<Compucolor::Logger::ILogger> _logger;
            std::shared_ptr<Compucolor::Common::IDisplay> _display;
            std::shared_ptr<Compucolor::Scheduler::IScheduler> _scheduler;
            std::shared_ptr<Compucolor::Memory::IMemory> _memory;
            std::shared_ptr<Compucolor::Smc5027::ISmc5027Emulator> _smc5027emulator;
            std::optional<std::thread> _loop;

            int _phase;

            /**
             * @brief  
             * @note   
             * @retval None
             */
            void RefreshDisplay();

            /**
             * @brief  
             * @note   
             * @retval None
             */
            void DrawCursor();

            /**
             * @brief  
             * @note   
             * @param  foreground: 
             * @param  background: 
             * @param  glyphData: 
             * @param  blink: 
             * @param  x: 
             * @param  y: 
             * @retval None
             */
            void DrawGlyph(
                Compucolor::Common::Color foreground,
                Compucolor::Common::Color background,
                uint8_t glyphData,
                bool blink,
                int x,
                int y
            );

            /**
             * @brief  
             * @note   
             * @retval 
             */
            bool IsBlinkOn();

            /**
             * @brief  
             * @note   
             * @param  bit: 
             * @param  data: 
             * @retval 
             */
            static bool IsBitSet(int bit, uint8_t data);

            /**
             * @brief  
             * @note   
             * @param  data: 
             * @retval 
             */
            static Compucolor::Common::Color GetForegroundColor(uint8_t data);

            /**
             * @brief  
             * @note   
             * @param  data: 
             * @retval 
             */
            static Compucolor::Common::Color GetBackgroundColor(uint8_t data);

            /**
             * @brief  
             * @note   
             * @param  data: 
             * @retval 
             */
            static Compucolor::Common::Color GetColor(uint8_t data);
    };
}