
#pragma once

#include <memory>
#include <vector>

#include "Compucolor.Keyboard/IKeyboardEmulator.h"

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
        KeyboardEmulator();

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

    private:
        std::vector<uint8_t> _kbMatrix;
};