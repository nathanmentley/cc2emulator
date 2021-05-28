#pragma once

#include <memory>

#include "../Intel8080/IIntel8080Emulator.h"
#include "../Memory/IMemory.h"

namespace Compucolor::Common
{
    class IPluginManager {
        public:
            virtual ~IPluginManager() {}

            virtual std::shared_ptr<Intel8080::IIntel8080Emulator> GetIntel8080() = 0;
            virtual std::shared_ptr<Memory::IMemory> GetMemory() = 0;
    };
}