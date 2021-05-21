#pragma once

#include <memory>
#include <optional>
#include <thread>

#include <Compucolor/Memory/IMemory.h>
#include <Compucolor/Crt/ICrtEmulator.h>
#include <Compucolor/Intel8080/IIntel8080Emulator.h>
#include <Compucolor/Keyboard/IKeyboardEmulator.h>
#include <Compucolor/Scheduler/IScheduler.h>
#include <Compucolor/Smc5027/ISmc5027Emulator.h>
#include <Compucolor/Tms5501/ITms5501Emulator.h>

#include <Compucolor/ICompucolorEmulator.h>

#include "CompucolorIntel8080Bus.h"
#include "Roms.h"

namespace Compucolor::Impl::CompucolorII
{
    void LoadRom(Compucolor::Memory::IMemory* memory, uint8_t* rom, uint16_t startPosition, uint16_t length);

    class CompucolorEmulator: public ICompucolorEmulator {
        public:
            CompucolorEmulator(
                std::shared_ptr<Compucolor::Crt::ICrtEmulator> crt,
                std::shared_ptr<Compucolor::Intel8080::IIntel8080Emulator> intel8080,
                std::shared_ptr<Compucolor::Keyboard::IKeyboardEmulator> keyboard,
                std::shared_ptr<Compucolor::Memory::IMemory> memory,
                std::shared_ptr<Compucolor::Scheduler::IScheduler> scheduler,
                std::shared_ptr<Compucolor::Smc5027::ISmc5027Emulator> smc5027,
                std::shared_ptr<Compucolor::Tms5501::ITms5501Emulator> tms5501
            );

            virtual void Start() override;
            virtual void Stop() override;

            virtual void OnKeyUp(Compucolor::Keyboard::CompucolorIIKey key) override;

            virtual void OnKeyDown(Compucolor::Keyboard::CompucolorIIKey key) override;

        private:
            std::shared_ptr<Compucolor::Crt::ICrtEmulator> _crt;
            std::shared_ptr<Compucolor::Intel8080::IIntel8080Emulator> _intel8080;
            std::shared_ptr<Compucolor::Keyboard::IKeyboardEmulator> _keyboard;
            std::shared_ptr<Compucolor::Memory::IMemory> _memory;
            std::shared_ptr<Compucolor::Scheduler::IScheduler> _scheduler;
            std::shared_ptr<Compucolor::Smc5027::ISmc5027Emulator> _smc5027;
            std::shared_ptr<Compucolor::Tms5501::ITms5501Emulator> _tms5501;

            std::optional<std::thread> _loop;
    };
}