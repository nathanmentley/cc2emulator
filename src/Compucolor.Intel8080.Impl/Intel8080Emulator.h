#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "Compucolor.Core/IMemory.h"
#include "Compucolor.Intel8080/IIntel8080Emulator.h"

#include "Intel8080.h"
#include "Intel8080EmulatorContext.h"

class Intel8080Emulator: public IIntel8080Emulator {
    public:
        Intel8080Emulator(
            std::shared_ptr<IMemory> memory
        );

        virtual void Start() override;

        virtual void Stop() override;

        virtual void SetBus(std::shared_ptr<IIntel8080Bus> intel8080Bus) override;

    private:
        std::unique_ptr<Intel8080EmulatorContext> _context;
        std::unique_ptr<i8080> _cpu;
        std::shared_ptr<IMemory> _memory;
        std::atomic<bool> _isRunning;
        std::thread _thread;
};