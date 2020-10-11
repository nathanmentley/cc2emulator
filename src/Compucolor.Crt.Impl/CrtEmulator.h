
#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <thread>

#include <Compucolor.Core/IDisplay.h>
#include <Compucolor.Core/IMemory.h>

#include <Compucolor.Smc5027/ISmc5027Emulator.h>

#include <Compucolor.Crt/ICrtEmulator.h>

#include <Compucolor.Crt.Impl/FontRom.h>

/**
 * @brief  
 * @note   
 * @retval None
 */
class CrtEmulator: public ICrtEmulator {
    public:
        /**
         * @brief  
         * @note   
         * @param  memory: 
         * @param  smc5027emulator: 
         * @retval 
         */
        CrtEmulator(
            std::shared_ptr<IMemory> memory,
            std::shared_ptr<ISmc5027Emulator> smc5027emulator
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

        /**
         * @brief  
         * @note   
         * @retval None
         */
        virtual void RefreshDisplay() override;

        /**
         * @brief  
         * @note   
         * @param  display: 
         * @retval None
         */
        virtual void SetDisplay(IDisplay* display) override;

    private:
        static int const Columns = 64;
        static int const Rows = 32;
        static int const CharacterWidth = 6;
        static int const CharacterHeight = 8;
        static int const Width = Columns * CharacterWidth;
        static int const Height = Rows * CharacterHeight;

        std::optional<IDisplay*> _display;
        std::shared_ptr<IMemory> _memory;
        std::shared_ptr<ISmc5027Emulator> _smc5027emulator;
        std::atomic<bool> _isRunning;
        std::thread _thread;

        int _phase;

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
        void DrawGlyph(Color foreground, Color background, uint8_t glyphData, bool blink, int x, int y);

        /**
         * @brief  
         * @note   
         * @param  bit: 
         * @param  data: 
         * @retval 
         */
        bool IsBitSet(int bit, uint8_t data);

        /**
         * @brief  
         * @note   
         * @param  data: 
         * @retval 
         */
        Color GetForegroundColor(uint8_t data);

        /**
         * @brief  
         * @note   
         * @param  data: 
         * @retval 
         */
        Color GetBackgroundColor(uint8_t data);
        
        /**
         * @brief  
         * @note   
         * @param  data: 
         * @retval 
         */
        Color GetColor(uint8_t data);

        /**
         * @brief  
         * @note   
         * @retval 
         */
        bool IsBlinkOn();
};