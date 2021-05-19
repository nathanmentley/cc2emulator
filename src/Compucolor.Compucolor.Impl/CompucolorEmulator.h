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
    void LoadRom(IMemory* memory, uint8_t* rom, uint16_t startPosition, uint16_t length);

    class CompucolorEmulator: public ICompucolorEmulator {
        public:
            CompucolorEmulator(
                std::shared_ptr<ICrtEmulator> crt,
                std::shared_ptr<IIntel8080Emulator> intel8080,
                std::shared_ptr<IKeyboardEmulator> keyboard,
                std::shared_ptr<IMemory> memory,
                std::shared_ptr<IScheduler> scheduler,
                std::shared_ptr<ISmc5027Emulator> smc5027,
                std::shared_ptr<ITms5501Emulator> tms5501
            );

            virtual void Start() override;
            virtual void Stop() override;

            virtual void SetDisplay(IDisplay* display) override;

            virtual void OnKeyUp(CompucolorIIKey key) override;

            virtual void OnKeyDown(CompucolorIIKey key) override;

        private:
            std::shared_ptr<ICrtEmulator> _crt;
            std::shared_ptr<IIntel8080Emulator> _intel8080;
            std::shared_ptr<IKeyboardEmulator> _keyboard;
            std::shared_ptr<IMemory> _memory;
            std::shared_ptr<IScheduler> _scheduler;
            std::shared_ptr<ISmc5027Emulator> _smc5027;
            std::shared_ptr<ITms5501Emulator> _tms5501;

            std::optional<std::thread> _loop;
    };
}