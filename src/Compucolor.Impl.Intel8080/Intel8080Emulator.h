#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include <Compucolor/Memory/IMemory.h>
#include <Compucolor/Intel8080/IIntel8080Emulator.h>

#include "Intel8080.h"
#include "Intel8080EmulatorContext.h"

namespace Compucolor::Impl::Intel8080
{
    class Intel8080Emulator: public Compucolor::Intel8080::IIntel8080Emulator {
        public:
            Intel8080Emulator(
                std::shared_ptr<Compucolor::Memory::IMemory> memory
            );

            virtual void Start() override;

            virtual void Stop() override;

            virtual void Reset() override;

            virtual void SetBus(std::shared_ptr<Compucolor::Intel8080::IIntel8080Bus> intel8080Bus) override;

            virtual void RegisterInterrupt(uint8_t opcode) override;

            virtual void Step() override;

        private:
            std::unique_ptr<Compucolor::Impl::Intel8080::Intel8080EmulatorContext> _context;
            std::unique_ptr<Compucolor::Impl::Intel8080::i8080> _cpu;
            std::shared_ptr<Compucolor::Memory::IMemory> _memory;

            static uint8_t ReadByte(void* userdata, uint16_t addr);
            static void WriteByte(void* userdata, uint16_t addr, uint8_t data);
            static uint8_t ReadPort(void* userdata, uint8_t port);
            static void WritePort(void* userdata, uint8_t port, uint8_t data);
    };
}