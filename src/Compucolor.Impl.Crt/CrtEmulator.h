
#pragma once

#include <chrono>
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
             * @param  logger:
             * @param  display:
             * @param  scheduler:
             * @param  memory:
             * @param  smc5027emulator:
             * @param  tms5501Emulator:
             * @retval
             */
            CrtEmulator(
                std::shared_ptr<Logger::ILogger> logger,
                std::shared_ptr<Common::IDisplay> display,
                std::shared_ptr<Scheduler::IScheduler> scheduler,
                std::shared_ptr<Memory::IMemory> memory,
                std::shared_ptr<Smc5027::ISmc5027Emulator> smc5027emulator,
                std::shared_ptr<Tms5501::ITms5501Emulator> tms5501Emulator
            );

            /**
             * @brief  
             * @note   
             * @retval None
             */
            void Start() override;

            /**
             * @brief  
             * @note   
             * @retval None
             */
            void Stop() override;

            static std::shared_ptr<ICrtEmulator> Creator(Common::IPluginManager* manager);

        private:
            static constexpr int Columns = 64;
            static constexpr int Rows = 32;
            static constexpr int CharacterWidth = 6;
            static constexpr int CharacterHeight = 8;
            static constexpr int Width = Columns * CharacterWidth;
            static constexpr int Height = Rows * CharacterHeight;

            std::shared_ptr<Logger::ILogger> _logger;
            std::shared_ptr<Common::IDisplay> _display;
            std::shared_ptr<Scheduler::IScheduler> _scheduler;
            std::shared_ptr<Memory::IMemory> _memory;
            std::shared_ptr<Smc5027::ISmc5027Emulator> _smc5027emulator;
            std::shared_ptr<Tms5501::ITms5501Emulator> _tms5501Emulator;
            std::shared_ptr<Scheduler::IScheduledTask> _loop;


            int _phase;

            /**
             * @brief  
             * @note   
             * @retval None
             */
            void RefreshDisplay() const;

            /**
             * @brief  
             * @note   
             * @retval None
             */
            void DrawCursor(uint8_t firstRow) const;

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
                Common::Color foreground,
                Common::Color background,
                uint8_t glyphData,
                bool blink,
                int x,
                int y
            ) const;

            /**
             * @brief  
             * @note   
             * @retval 
             */
            [[nodiscard]] bool IsBlinkOn() const;

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
            static Common::Color GetForegroundColor(uint8_t data);

            /**
             * @brief  
             * @note   
             * @param  data: 
             * @retval 
             */
            static Common::Color GetBackgroundColor(uint8_t data);

            /**
             * @brief  
             * @note   
             * @param  data: 
             * @retval 
             */
            static Common::Color GetColor(uint8_t data);
    };
}