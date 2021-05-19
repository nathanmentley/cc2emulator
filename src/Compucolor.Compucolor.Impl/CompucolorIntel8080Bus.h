#pragma once

#include <memory>

#include <Compucolor/Intel8080/IIntel8080Bus.h>
#include <Compucolor/Smc5027/ISmc5027Emulator.h>
#include <Compucolor/Tms5501/ITms5501Emulator.h>

namespace Compucolor::Compucolor::Impl
{
    class CompucolorIntel8080Bus: public IIntel8080Bus
    {
        public:
            CompucolorIntel8080Bus(
                std::shared_ptr<ISmc5027Emulator> smc5027,
                std::shared_ptr<ITms5501Emulator> tms5501
            );
    
            virtual uint8_t Read(uint8_t port) override;
    
            virtual void Write(uint8_t port, uint8_t data) override;
    
        private:
            std::shared_ptr<ISmc5027Emulator> _smc5027;
            std::shared_ptr<ITms5501Emulator> _tms5501;
    };
}