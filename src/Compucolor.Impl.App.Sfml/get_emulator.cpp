#include "ThirdParty/boost/di.hpp"

#include "get_emulator.h"

std::unique_ptr<Compucolor::CompucolorEmulatorProcessor> Compucolor::Impl::App::Sfml::get_emulator(
    std::shared_ptr<Compucolor::Logger::ILoggerProvider> loggerProvider,
    std::shared_ptr<Compucolor::Common::IDisplay> display
)
{
    return boost::di::make_injector(
        boost::di::bind<class Compucolor::Logger::ILoggerProvider>.to(loggerProvider),
        boost::di::bind<class Compucolor::Common::IDisplay>.to(display),
        boost::di::bind<class Compucolor::Logger::ILogger>.to<Impl::Logger::Logger>(),
        boost::di::bind<class Compucolor::Configuration::IConfiguration>.to<Impl::Configuration::Configuration>(),
        boost::di::bind<class Compucolor::Scheduler::IScheduler>.to<Impl::Scheduler::Scheduler>(),
        boost::di::bind<class Compucolor::Memory::IMemory>.to<Impl::Memory::VectorMemory>(),
        boost::di::bind<class Compucolor::Intel8080::IIntel8080Emulator>.to<Impl::Intel8080::Intel8080Emulator>(),
        boost::di::bind<class Compucolor::Smc5027::ISmc5027Emulator>.to<Impl::Smc5027::Smc5027Emulator>(),
        boost::di::bind<class Compucolor::Keyboard::IKeyboardEmulator>.to<Impl::Keyboard::KeyboardEmulator>(),
        boost::di::bind<class Compucolor::Floppy::IFloppyEmulator>.to<Impl::Floppy::FloppyEmulator>(),
        boost::di::bind<class Compucolor::Tms5501::ITms5501Emulator>.to<Impl::Tms5501::Tms5501Emulator>(),
        boost::di::bind<class Compucolor::Crt::ICrtEmulator>.to<Impl::Crt::CrtEmulator>(),
        boost::di::bind<class Compucolor::ICompucolorEmulator>.to<Impl::CompucolorII::CompucolorEmulator>()
    )
        .create<std::unique_ptr<CompucolorEmulatorProcessor>>();
}