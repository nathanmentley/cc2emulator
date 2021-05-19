#include <Compucolor.App/ThirdParty/boost/di.hpp>

#include <Compucolor.App/get_emulator.h>

std::unique_ptr<ICompucolorEmulator> Compucolor::App::get_emulator(
    std::shared_ptr<ILoggerProvider> loggerProvider
)
{
    return boost::di::make_injector(
        boost::di::bind<class ILoggerProvider>.to(loggerProvider),
        boost::di::bind<class ILogger>.to<Logger::Impl::Logger>(),
        boost::di::bind<class Configuration::IConfiguration>.to<Configuration::Impl::Configuration>(),
        boost::di::bind<class IScheduler>.to<Scheduler::Impl::Scheduler>(),
        boost::di::bind<class IMemory>.to<Memory::Impl::VectorMemory>(),
        boost::di::bind<class Intel8080::IIntel8080Emulator>.to<Intel8080::Impl::Intel8080Emulator>(),
        boost::di::bind<class ISmc5027Emulator>.to<Smc5027::Impl::Smc5027Emulator>(),
        boost::di::bind<class IKeyboardEmulator>.to<Keyboard::Impl::KeyboardEmulator>(),
        boost::di::bind<class Floppy::IFloppyEmulator>.to<Floppy::Impl::FloppyEmulator>(),
        boost::di::bind<class ITms5501Emulator>.to<Tms5501::Impl::Tms5501Emulator>(),
        boost::di::bind<class Crt::ICrtEmulator>.to<Crt::Impl::CrtEmulator>()
    )
        .create<std::unique_ptr<Compucolor::Impl::CompucolorEmulator>>();
}