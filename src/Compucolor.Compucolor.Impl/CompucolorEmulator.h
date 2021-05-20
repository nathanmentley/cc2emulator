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

namespace Compucolor::Compucolor::Impl
{
    void LoadRom(Memory::IMemory* memory, uint8_t* rom, uint16_t startPosition, uint16_t length);

    class CompucolorEmulator: public ICompucolorEmulator {
        public:
            CompucolorEmulator(
                std::shared_ptr<Crt::ICrtEmulator> crt,
                std::shared_ptr<Intel8080::IIntel8080Emulator> intel8080,
                std::shared_ptr<Keyboard::IKeyboardEmulator> keyboard,
                std::shared_ptr<Memory::IMemory> memory,
                std::shared_ptr<Scheduler::IScheduler> scheduler,
                std::shared_ptr<Smc5027::ISmc5027Emulator> smc5027,
                std::shared_ptr<Tms5501::ITms5501Emulator> tms5501
            );

            virtual void Start() override;
            virtual void Stop() override;

            virtual void SetDisplay(Common::IDisplay* display) override;

            virtual void OnKeyUp(Keyboard::CompucolorIIKey key) override;

            virtual void OnKeyDown(Keyboard::CompucolorIIKey key) override;

        private:
            std::shared_ptr<Crt::ICrtEmulator> _crt;
            std::shared_ptr<Intel8080::IIntel8080Emulator> _intel8080;
            std::shared_ptr<Keyboard::IKeyboardEmulator> _keyboard;
            std::shared_ptr<Memory::IMemory> _memory;
            std::shared_ptr<Scheduler::IScheduler> _scheduler;
            std::shared_ptr<Smc5027::ISmc5027Emulator> _smc5027;
            std::shared_ptr<Tms5501::ITms5501Emulator> _tms5501;

            std::optional<std::thread> _loop;
    };
}