#pragma once

#include <memory>

#include <Compucolor/Common/IDisplay.h>

#include <Compucolor/Memory/IMemory.h>
#include <Compucolor.Memory.Impl/VectorMemory.h>

#include <Compucolor/Configuration/IConfiguration.h>
#include <Compucolor.Configuration.Impl/Configuration.h>

#include <Compucolor/Floppy/IFloppyEmulator.h>
#include <Compucolor.Floppy.Impl/FloppyEmulator.h>

#include <Compucolor/Intel8080/IIntel8080Emulator.h>
#include <Compucolor.Intel8080.Impl/Intel8080Emulator.h>

#include <Compucolor/Smc5027/ISmc5027Emulator.h>
#include <Compucolor.Smc5027.Impl/Smc5027Emulator.h>

#include <Compucolor/Tms5501/ITms5501Emulator.h>
#include <Compucolor.Tms5501.Impl/Tms5501Emulator.h>

#include <Compucolor/Keyboard/IKeyboardEmulator.h>
#include <Compucolor.Keyboard.Impl/KeyboardEmulator.h>

#include <Compucolor/Crt/ICrtEmulator.h>
#include <Compucolor.Crt.Impl/CrtEmulator.h>

#include <Compucolor/Scheduler/IScheduler.h>
#include <Compucolor.Scheduler.Impl/Scheduler.h>

#include <Compucolor/Logger/ILogger.h>
#include <Compucolor.Logger.Impl/Logger.h>

#include <Compucolor/ICompucolorEmulator.h>
#include <Compucolor.Compucolor.Impl/CompucolorEmulator.h>

#include <Compucolor/EmulatorBackgroundTask.h>

namespace Compucolor::App
{
    std::unique_ptr<EmulatorBackgroundTask> get_emulator(
        std::shared_ptr<Logger::ILoggerProvider> loggerProvider,
        std::shared_ptr<Common::IDisplay> display
    );
}