#include <Compucolor.Core/ThirdParty/boost/di.hpp>

#include <Compucolor.App/get_emulator.h>

std::unique_ptr<ICompucolorEmulator> get_emulator(
    std::shared_ptr<ILoggerProvider> loggerProvider
)
{
    return boost::di::make_injector(
        boost::di::bind<class ILoggerProvider>.to(loggerProvider),
        boost::di::bind<class ILogger>.to<Logger>(),
        boost::di::bind<class IConfiguration>.to<Configuration>(),
        boost::di::bind<class IScheduler>.to<Scheduler>(),
        boost::di::bind<class IMemory>.to<VectorMemory>(),
        boost::di::bind<class IIntel8080Emulator>.to<Intel8080Emulator>(),
        boost::di::bind<class ISmc5027Emulator>.to<Compucolor::Smc5027::Impl::Smc5027Emulator>(),
        boost::di::bind<class IKeyboardEmulator>.to<KeyboardEmulator>(),
        boost::di::bind<class IFloppyEmulator>.to<FloppyEmulator>(),
        boost::di::bind<class ITms5501Emulator>.to<Tms5501Emulator>(),
        boost::di::bind<class ICrtEmulator>.to<CrtEmulator>()
    )
        .create<std::unique_ptr<CompucolorEmulator>>();
}