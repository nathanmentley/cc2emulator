
#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include <Compucolor/Keyboard/IKeyboardEmulator.h>
#include <Compucolor/Logger/ILogger.h>

namespace Compucolor::Keyboard::Impl
{
    /**
     * @brief  
     * @note   
     * @retval None
     */
    class KeyboardEmulator: public IKeyboardEmulator {
        public:
            /**
             * @brief  
             * @note   
             * @param  memory: 
             * @retval 
             */
            KeyboardEmulator(
                std::shared_ptr<Logger::ILogger> logger
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
             * @param  port: 
             * @retval 
             */
            virtual uint8_t Read(uint8_t port) override;

            /**
             * @brief  
             * @note   
             * @retval None
             */
            virtual void Reset() override;

            /**
             * @brief  
             * @note   
             * @param  key: 
             * @retval None
             */
            virtual void OnKeyUp(CompucolorIIKey key) override;

            /**
             * @brief  
             * @note   
             * @param  key: 
             * @retval None
             */
            virtual void OnKeyDown(CompucolorIIKey key) override;

        private:
            std::shared_ptr<Logger::ILogger> _logger;
            std::vector<uint8_t> _kbMatrix;

            static std::optional<int> GetRow(CompucolorIIKey key);

            static std::optional<int> GetBit(CompucolorIIKey key);
    };
}