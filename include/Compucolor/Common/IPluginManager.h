#pragma once

#include <memory>

#include <Compucolor/Common/IDisplay.h>
#include <Compucolor/ICompucolorEmulator.h>
#include <Compucolor/Intel8080/IIntel8080Emulator.h>
#include <Compucolor/Logger/ILoggerProvider.h>
#include <Compucolor/Logger/ILogger.h>
#include <Compucolor/Crt/ICrtEmulator.h>
#include <Compucolor/Memory/IMemory.h>
#include <Compucolor/Scheduler/IScheduler.h>
#include <Compucolor/Keyboard/IKeyboardEmulator.h>
#include <Compucolor/Floppy/IFloppyEmulator.h>
#include <Compucolor/Smc5027/ISmc5027Emulator.h>
#include <Compucolor/Tms5501/ITms5501Emulator.h>

namespace Compucolor::Common
{
    class IPluginManager {
        public:
            virtual ~IPluginManager() = default;

            virtual std::shared_ptr<Logger::ILoggerProvider> GetLoggerProvider() = 0;
            virtual std::shared_ptr<IDisplay> GetDisplay() = 0;
            virtual std::shared_ptr<Logger::ILogger> GetLogger() = 0;
            virtual std::shared_ptr<Scheduler::IScheduler> GetScheduler() = 0;
            virtual std::shared_ptr<Intel8080::IIntel8080Emulator> GetIntel8080() = 0;
            virtual std::shared_ptr<Memory::IMemory> GetMemory() = 0;
            virtual std::shared_ptr<Keyboard::IKeyboardEmulator> GetKeyboard() = 0;
            virtual std::shared_ptr<Floppy::IFloppyEmulator> GetFloppyA() = 0;
            virtual std::shared_ptr<Floppy::IFloppyEmulator> GetFloppyB() = 0;
            virtual std::shared_ptr<Smc5027::ISmc5027Emulator> GetSmc5027() = 0;
            virtual std::shared_ptr<Crt::ICrtEmulator> GetCrt() = 0;
            virtual std::shared_ptr<Tms5501::ITms5501Emulator> GetTms5501() = 0;
            virtual std::shared_ptr<ICompucolorEmulator> GetCompucolor() = 0;
    };
}