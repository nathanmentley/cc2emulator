
#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <optional>

#include <Compucolor/Common/IPluginManager.h>
#include <Compucolor/Keyboard/IKeyboardEmulator.h>
#include <Compucolor/Logger/ILogger.h>

namespace Compucolor::Impl::Keyboard
{
    /**
     * @brief  
     * @note   
     * @retval None
     */
    class KeyboardEmulator: public Compucolor::Keyboard::IKeyboardEmulator {
        public:
            /**
             * @brief  
             * @note   
             * @param  logger:
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
            void Start() override;

            /**
             * @brief  
             * @note   
             * @retval None
             */
            void Stop() override;

            /**
             * @brief  
             * @note   
             * @param  port: 
             * @retval 
             */
            uint8_t Read(uint8_t port) override;

            /**
             * @brief  
             * @note   
             * @retval None
             */
            void Reset() override;

            /**
             * @brief  
             * @note   
             * @param  key: 
             * @retval None
             */
            void OnKeyUp(Compucolor::Keyboard::CompucolorIIKey key) override;

            /**
             * @brief  
             * @note   
             * @param  key: 
             * @retval None
             */
            void OnKeyDown(Compucolor::Keyboard::CompucolorIIKey key) override;

            static std::shared_ptr<Compucolor::Keyboard::IKeyboardEmulator> Creator(Common::IPluginManager* manager);

        private:
            std::shared_ptr<Logger::ILogger> _logger;
            std::array<std::atomic<uint8_t>, 17> _kbMatrix{};

            static std::optional<int> GetRow(Compucolor::Keyboard::CompucolorIIKey key);

            static std::optional<int> GetBit(Compucolor::Keyboard::CompucolorIIKey key);
    };
}