#pragma once

#include <memory>
#include <optional>

#include <Compucolor/Memory/IMemory.h>
#include <Compucolor/Intel8080/IIntel8080Bus.h>

namespace Compucolor::Intel8080::Impl
{
    class Intel8080EmulatorContext {
        public:
            Intel8080EmulatorContext();
    
            void SetMemory(std::shared_ptr<Compucolor::Memory::IMemory> memory);
            void SetBus(std::shared_ptr<IIntel8080Bus> intel8080Bus);
    
            std::optional<std::shared_ptr<Compucolor::Memory::IMemory>> GetMemory();
            std::optional<std::shared_ptr<IIntel8080Bus>> GetBus();
    
        private:
            std::optional<std::shared_ptr<IIntel8080Bus>> _bus;
            std::optional<std::shared_ptr<Compucolor::Memory::IMemory>> _memory;
    };
}