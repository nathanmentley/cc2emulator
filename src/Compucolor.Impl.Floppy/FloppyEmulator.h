#pragma once

#include <memory>

#include <Compucolor/Common/IPluginManager.h>
#include <Compucolor/Floppy/IFloppyEmulator.h>

namespace Compucolor::Impl::Floppy
{
    /**
     * @brief  
     * @note   
     */
    class FloppyEmulator: public Compucolor::Floppy::IFloppyEmulator {
        public:
            /**
             * @brief  
             * @note   
             * @retval
             */
            FloppyEmulator(std::shared_ptr<Logger::ILogger> logger);

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
             * @retval None
             */
            void Reset() override;

            /**
             * @brief  
             * @note   
             * @param  port: 
             * @retval None
             */
            uint8_t Read(uint8_t port) override;

            /**
             * @brief  
             * @note   
             * @param  port: 
             * @param  data: 
             * @retval None
             */
            void Write(uint8_t port, uint8_t data) override;

            /**
             * @brief  
             * @note   
             * @param  selected: 
             * @param  write: 
             * @param  stepper: 
             * @retval None
             */
            void Select(bool selected, bool write, uint8_t stepper) override;

            void ReadFile(std::string filename) override;

            void SaveFile(std::string filename) override;

            static std::shared_ptr<IFloppyEmulator> Creator(Common::IPluginManager* manager);
        private:
            std::shared_ptr<Logger::ILogger> _logger;
            bool _selected = false;
            bool _writeEnable = false;
            uint8_t _stepperPhase = 0;
            int _track = 0;
            size_t _bytePos = 0;
            std::vector<uint8_t> _disk;

            void StepHead(uint8_t newPhase);
    };
}