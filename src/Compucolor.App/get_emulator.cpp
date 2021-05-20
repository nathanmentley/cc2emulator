#include <Compucolor.App/ThirdParty/boost/di.hpp>

#include <Compucolor.App/get_emulator.h>

std::unique_ptr<Compucolor::EmulatorBackgroundTask> Compucolor::App::get_emulator(
    std::shared_ptr<Logger::ILoggerProvider> loggerProvider,
    std::shared_ptr<Common::IDisplay> display
)
{
    return boost::di::make_injector(
        boost::di::bind<class Logger::ILoggerProvider>.to(loggerProvider),
        boost::di::bind<class Common::IDisplay>.to(display),
        boost::di::bind<class Logger::ILogger>.to<Logger::Impl::Logger>(),
        boost::di::bind<class Configuration::IConfiguration>.to<Configuration::Impl::Configuration>(),
        boost::di::bind<class Scheduler::IScheduler>.to<Scheduler::Impl::Scheduler>(),
        boost::di::bind<class Memory::IMemory>.to<Memory::Impl::VectorMemory>(),
        boost::di::bind<class Intel8080::IIntel8080Emulator>.to<Intel8080::Impl::Intel8080Emulator>(),
        boost::di::bind<class Smc5027::ISmc5027Emulator>.to<Smc5027::Impl::Smc5027Emulator>(),
        boost::di::bind<class Keyboard::IKeyboardEmulator>.to<Keyboard::Impl::KeyboardEmulator>(),
        boost::di::bind<class Floppy::IFloppyEmulator>.to<Floppy::Impl::FloppyEmulator>(),
        boost::di::bind<class Tms5501::ITms5501Emulator>.to<Tms5501::Impl::Tms5501Emulator>(),
        boost::di::bind<class Crt::ICrtEmulator>.to<Crt::Impl::CrtEmulator>(),
        boost::di::bind<class ICompucolorEmulator>.to<Compucolor::Impl::CompucolorEmulator>()
    )
        .create<std::unique_ptr<EmulatorBackgroundTask>>();
}