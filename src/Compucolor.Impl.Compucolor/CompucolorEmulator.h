#pragma once

#include <cstdint>
#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <thread>

#include <Compucolor/Common/IPluginManager.h>
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
    void LoadRom(Memory::IMemory* memory, const uint8_t* rom, uint16_t startPosition, uint16_t length);

    class CompucolorEmulator: public ICompucolorEmulator {
        public:
            CompucolorEmulator(
                std::shared_ptr<Crt::ICrtEmulator> crt,
                std::shared_ptr<Floppy::IFloppyEmulator> floppyA,
                std::shared_ptr<Floppy::IFloppyEmulator> floppyB,
                std::shared_ptr<Intel8080::IIntel8080Emulator> intel8080,
                std::shared_ptr<Keyboard::IKeyboardEmulator> keyboard,
                std::shared_ptr<Memory::IMemory> memory,
                std::shared_ptr<Scheduler::IScheduler> scheduler,
                std::shared_ptr<Smc5027::ISmc5027Emulator> smc5027,
                std::shared_ptr<Tms5501::ITms5501Emulator> tms5501,
                std::function<uint64_t()> clock = nullptr
            );

            void Start() override;
            void Stop() override;

            void OnKeyUp(Keyboard::CompucolorIIKey key) override;

            void OnKeyDown(Keyboard::CompucolorIIKey key) override;

            void Reset(bool hard) override;
            void Pause() override;
            void Resume() override;
            [[nodiscard]] bool IsPaused() const override;
            void SetSpeedRegulated(bool regulated) override;
            [[nodiscard]] bool IsSpeedRegulated() const override;
            void SetLowercaseCharset(bool lowercase) override;
            void SetRomVersion(RomVersion version) override;
            [[nodiscard]] std::shared_ptr<Floppy::IFloppyEmulator> GetFloppy(int drive) const override;

            static std::shared_ptr<ICompucolorEmulator> Creator(Common::IPluginManager* manager);

        private:
            std::shared_ptr<Crt::ICrtEmulator> _crt;
            std::shared_ptr<Floppy::IFloppyEmulator> _floppyA;
            std::shared_ptr<Floppy::IFloppyEmulator> _floppyB;
            std::shared_ptr<Intel8080::IIntel8080Emulator> _intel8080;
            std::shared_ptr<Keyboard::IKeyboardEmulator> _keyboard;
            std::shared_ptr<Memory::IMemory> _memory;
            std::shared_ptr<Scheduler::IScheduler> _scheduler;
            std::shared_ptr<Smc5027::ISmc5027Emulator> _smc5027;
            std::shared_ptr<Tms5501::ITms5501Emulator> _tms5501;
            std::shared_ptr<Scheduler::IScheduledTask> _loop;
            int _loopInterval = 0;
            uint64_t _floppyCycleRemainder = 0;

            std::function<uint64_t()> _clock;
            uint64_t _lastTickNanoseconds = 0;
            int64_t _budgetNanoseconds = 0;

            static constexpr int ResetNone = 0;
            static constexpr int ResetWarm = 1;
            static constexpr int ResetHard = 2;
            static constexpr int UnregulatedBurst = 4096;
            static constexpr uint64_t CycleNanoseconds = 502;
            static constexpr uint64_t TickNanoseconds = 1'000'000;
            // Caps catch-up after a stall so the emulator never spirals trying to make up lost time.
            static constexpr int64_t MaxBudgetNanoseconds = 50'000'000;
            static constexpr uint16_t PowerUpFlagAddress = 0x81AF + 8;

            std::atomic<int> _resetRequest{ResetNone};
            std::atomic<bool> _paused{false};
            std::atomic<bool> _speedRegulated{true};
            std::atomic<RomVersion> _romVersion{RomVersion::V8_79};

            uint64_t StepOnce();
            void LoadSystemRom();
            void ApplyReset(bool hard);
    };
}