
#pragma once

#include <memory>

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IResetable.h>

#include "IIntel8080Bus.h"

namespace Compucolor::Intel8080
{
    class IIntel8080Emulator: public Common::IEmulatable, public Common::IResetable {
        public:
            virtual ~IIntel8080Emulator() {}

            virtual void SetBus(std::shared_ptr<IIntel8080Bus> intel8080Bus) = 0;

            virtual void RegisterInterrupt(uint8_t opcode) = 0;

            virtual void Step() = 0;
    };
}