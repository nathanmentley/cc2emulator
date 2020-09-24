
#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include <fruit/fruit.h>

#include "Compucolor.Core/IMemory.h"
#include "Compucolor.Intel8080/IIntel8080Emulator.h"

#include "Intel8080.h"

class Intel8080Emulator: public IIntel8080Emulator {
    public:
        INJECT(
            Intel8080Emulator(
                std::shared_ptr<IMemory> memory
            )
        );

        virtual void Start() override;

        virtual void Stop() override;

    private:
        std::unique_ptr<i8080> _cpu;
        std::shared_ptr<IMemory> _memory;
        std::atomic<bool> _isRunning;
        std::thread _thread;
};