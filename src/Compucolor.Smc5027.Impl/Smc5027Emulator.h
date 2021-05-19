#pragma once

#include <vector>

#include <Compucolor/Logger/ILogger.h>

#include <Compucolor/Smc5027/ISmc5027Emulator.h>

namespace Compucolor::Smc5027::Impl {
    /**
     * @brief  
     * @note   
     * @retval None
     */
    class Smc5027Emulator: public ISmc5027Emulator {
        public:
            /**
             * @brief  
             * @note   
             * @retval 
             */
            Smc5027Emulator(
                std::shared_ptr<ILogger> logger
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
             * @param  port: 
             * @param  data: 
             * @retval None
             */
            virtual void Write(uint8_t port, uint8_t data) override;
    
            /**
             * @brief  
             * @note   
             * @retval None
             */
            virtual void Reset() override;
    
            /**
             * @brief  
             * @note   
             * @retval 
             */
            virtual uint8_t GetCursorX() override;
    
            /**
             * @brief  
             * @note   
             * @retval 
             */
            virtual uint8_t GetCursorY() override;
        private:
            static const uint8_t REGISTER_COUNT = 16;
    
            std::shared_ptr<ILogger> _logger;
            std::vector<uint8_t> _registers;
    
            /**
             * @brief  
             * @note   
             * @param  port: 
             * @retval 
             */
            uint8_t ConvertPort(uint8_t port);
    };
}