#include <Compucolor.App/ThirdParty/boost/di.hpp>

#include <Compucolor.App/get_emulator.h>

std::unique_ptr<Compucolor::EmulatorBackgroundTask> Compucolor::App::get_emulator(
    std::shared_ptr<Compucolor::Logger::ILoggerProvider> loggerProvider,
    std::shared_ptr<Compucolor::Common::IDisplay> display
)
{
    return boost::di::make_injector(
        boost::di::bind<class Logger::ILoggerProvider>.to(loggerProvider),
        boost::di::bind<class Common::IDisplay>.to(display),
        boost::di::bind<class Logger::ILogger>.to<Impl::Logger::Logger>(),
        boost::di::bind<class Configuration::IConfiguration>.to<Impl::Configuration::Configuration>(),
        boost::di::bind<class Scheduler::IScheduler>.to<Impl::Scheduler::Scheduler>(),
        boost::di::bind<class Memory::IMemory>.to<Impl::Memory::VectorMemory>(),
        boost::di::bind<class Intel8080::IIntel8080Emulator>.to<Impl::Intel8080::Intel8080Emulator>(),
        boost::di::bind<class Smc5027::ISmc5027Emulator>.to<Impl::Smc5027::Smc5027Emulator>(),
        boost::di::bind<class Keyboard::IKeyboardEmulator>.to<Impl::Keyboard::KeyboardEmulator>(),
        boost::di::bind<class Floppy::IFloppyEmulator>.to<Impl::Floppy::FloppyEmulator>(),
        boost::di::bind<class Tms5501::ITms5501Emulator>.to<Impl::Tms5501::Tms5501Emulator>(),
        boost::di::bind<class Crt::ICrtEmulator>.to<Impl::Crt::CrtEmulator>(),
        boost::di::bind<class ICompucolorEmulator>.to<Impl::CompucolorII::CompucolorEmulator>()
    )
        .create<std::unique_ptr<EmulatorBackgroundTask>>();
}