
#pragma once

#include <memory>

#include "Compucolor.Core/IEmulatable.h"
#include "Compucolor.Core/IResetable.h"

#include "IIntel8080Bus.h"

class IIntel8080Emulator: public IEmulatable, public IResetable {
    public:
        virtual ~IIntel8080Emulator() {}

        virtual void SetBus(std::shared_ptr<IIntel8080Bus> intel8080Bus) = 0;

        virtual void RegisterInterrupt(uint8_t opcode) = 0;
};