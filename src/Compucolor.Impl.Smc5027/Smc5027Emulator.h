#pragma once

#include <array>
#include <atomic>

#include <Compucolor/Common/IPluginManager.h>
#include <Compucolor/Logger/ILogger.h>
#include <Compucolor/Smc5027/ISmc5027Emulator.h>

namespace Compucolor::Impl::Smc5027 {
    /**
     * @brief  
     * @note   
     * @retval None
     */
    class Smc5027Emulator: public Compucolor::Smc5027::ISmc5027Emulator {
        public:
            /**
             * @brief  
             * @note   
             * @retval 
             */
            Smc5027Emulator(
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

            /**
             * @brief  
             * @note   
             * @retval 
             */
            virtual uint8_t FirstDisplayRow() override;

            static std::shared_ptr<Compucolor::Smc5027::ISmc5027Emulator> Creator(Common::IPluginManager* manager);
        private:
            static constexpr uint8_t REGISTER_COUNT = 16;
            static constexpr uint8_t ROW_COUNT = 32;
    
            std::shared_ptr<Logger::ILogger> _logger;
            std::array<std::atomic<uint8_t>, REGISTER_COUNT> _registers{};
    
            /**
             * @brief  
             * @note   
             * @param  port: 
             * @retval 
             */
            static uint8_t ConvertPort(uint8_t port);
    };
}