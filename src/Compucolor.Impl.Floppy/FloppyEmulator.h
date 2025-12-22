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
             * @param  memory: 
             * @retval 
             */
            FloppyEmulator();

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
            virtual void Reset() override;

            /**
             * @brief  
             * @note   
             * @param  port: 
             * @retval None
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
             * @param  selected: 
             * @param  write: 
             * @param  stepper: 
             * @retval None
             */
            virtual void Select(bool selected, bool write, uint8_t stepper) override;

            static std::shared_ptr<Compucolor::Floppy::IFloppyEmulator> Creator(Common::IPluginManager* manager);
        private:
            /**
             * @brief  
             * @note   
             */
            static const int StepperPhases = 3;//4;
    };
}