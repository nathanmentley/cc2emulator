#pragma once

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IOutputDevice.h>
#include <Compucolor/Common/IResetable.h>

/**
 * @brief  
 * @note   
 * @retval None
 */
class ISmc5027Emulator: public IEmulatable, public IInputDevice, public IOutputDevice, public IResetable {
    public:
        /**
         * @brief  
         * @note   
         * @retval None
         */
        virtual ~ISmc5027Emulator() {}

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
};