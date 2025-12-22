#pragma once

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IOutputDevice.h>
#include <Compucolor/Common/IResetable.h>
#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Smc5027
{
    /**
     * @brief  
     * @note   
     * @retval None
     */
    class ISmc5027Emulator: public Common::IPlugin, public Common::IEmulatable, public Common::IInputDevice, public Common::IOutputDevice, public Common::IResetable {
        public:
            /**
             * @brief  
             * @note   
             * @retval None
             */
            ~ISmc5027Emulator() override = default;

            /**
             * @brief  
             * @note   
             * @retval 
             */
            virtual uint8_t GetCursorX() = 0;

            /**
             * @brief  
             * @note   
             * @retval 
             */
            virtual uint8_t GetCursorY() = 0;

            /**
             * @brief  
             * @note   
             * @retval 
             */
            virtual uint8_t FirstDisplayRow() = 0;
    };
}