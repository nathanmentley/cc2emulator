#pragma once

#include <memory>

#include <Compucolor/Common/IDisplay.h>

#include <Compucolor/Memory/IMemory.h>

#include <Compucolor/Configuration/IConfiguration.h>
#include <Compucolor.Impl.Configuration/Configuration.h>

#include <Compucolor/Floppy/IFloppyEmulator.h>
#include <Compucolor.Impl.Floppy/FloppyEmulator.h>

#include <Compucolor/Intel8080/IIntel8080Emulator.h>
#include <Compucolor.Impl.Intel8080/Intel8080Emulator.h>

#include <Compucolor/Smc5027/ISmc5027Emulator.h>
#include <Compucolor.Impl.Smc5027/Smc5027Emulator.h>

#include <Compucolor/Tms5501/ITms5501Emulator.h>
#include <Compucolor.Impl.Tms5501/Tms5501Emulator.h>

#include <Compucolor/Keyboard/IKeyboardEmulator.h>
#include <Compucolor.Impl.Keyboard/KeyboardEmulator.h>

#include <Compucolor/Crt/ICrtEmulator.h>
#include <Compucolor.Impl.Crt/CrtEmulator.h>

#include <Compucolor/Scheduler/IScheduler.h>
#include <Compucolor.Impl.Scheduler/Scheduler.h>

#include <Compucolor/Logger/ILogger.h>
#include <Compucolor.Impl.Logger/Logger.h>

#include <Compucolor/ICompucolorEmulator.h>
#include <Compucolor.Impl.Compucolor/CompucolorEmulator.h>

#include <Compucolor/Common/PluginLoader.h>

#include <Compucolor/CompucolorEmulatorProcessor.h>

namespace Compucolor::Impl::App::Sfml
{
    std::unique_ptr<CompucolorEmulatorProcessor> get_emulator(
        std::shared_ptr<Compucolor::Logger::ILoggerProvider> loggerProvider,
        std::shared_ptr<Compucolor::Common::IDisplay> display
    );
}