#pragma once

#include <memory>

#include "./IDisplay.h"
#include "../ICompucolorEmulator.h"
#include "../Intel8080/IIntel8080Emulator.h"
#include "../Logger/ILoggerProvider.h"
#include "../Logger/ILogger.h"
#include "../Crt/ICrtEmulator.h"
#include "../Memory/IMemory.h"
#include "../Scheduler/IScheduler.h"
#include "../Keyboard/IKeyboardEmulator.h"
#include "../Floppy/IFloppyEmulator.h"
#include "../Smc5027/ISmc5027Emulator.h"
#include "../Tms5501/ITms5501Emulator.h"

namespace Compucolor::Common
{
    class IPluginManager {
        public:
            virtual ~IPluginManager() {}

            virtual std::shared_ptr<Logger::ILoggerProvider> GetLoggerProvider() = 0;
            virtual std::shared_ptr<IDisplay> GetDisplay() = 0;
            virtual std::shared_ptr<Logger::ILogger> GetLogger() = 0;
            virtual std::shared_ptr<Scheduler::IScheduler> GetScheduler() = 0;
            virtual std::shared_ptr<Intel8080::IIntel8080Emulator> GetIntel8080() = 0;
            virtual std::shared_ptr<Memory::IMemory> GetMemory() = 0;
            virtual std::shared_ptr<Keyboard::IKeyboardEmulator> GetKeyboard() = 0;
            virtual std::shared_ptr<Floppy::IFloppyEmulator> GetFloppy() = 0;
            virtual std::shared_ptr<Smc5027::ISmc5027Emulator> GetSmc5027() = 0;
            virtual std::shared_ptr<Crt::ICrtEmulator> GetCrt() = 0;
            virtual std::shared_ptr<Tms5501::ITms5501Emulator> GetTms5501() = 0;
            virtual std::shared_ptr<ICompucolorEmulator> GetCompucolor() = 0;
    };
}