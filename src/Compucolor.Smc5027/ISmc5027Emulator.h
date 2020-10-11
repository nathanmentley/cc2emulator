#pragma once

#include <Compucolor.Core/IEmulatable.h>
#include <Compucolor.Core/IInputDevice.h>
#include <Compucolor.Core/IOutputDevice.h>
#include <Compucolor.Core/IResetable.h>

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