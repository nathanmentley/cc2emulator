#include <Compucolor.App/get_emulator.h>

std::unique_ptr<ICompucolorEmulator> get_emulator(
    std::shared_ptr<ILoggerProvider> loggerProvider
)
{
    std::shared_ptr<ILogger> logger =
        std::shared_ptr<ILogger>(new Logger());

    logger->AddProvider(loggerProvider);

    std::shared_ptr<IConfiguration> configuration =
        std::shared_ptr<IConfiguration>(new Configuration(logger));

    configuration->LoadFile("app.config");

    std::shared_ptr<IMemory> memory =
        std::shared_ptr<IMemory>(new ByteArrayMemory());

    std::shared_ptr<IIntel8080Emulator> intel8080Emulator =
        std::shared_ptr<IIntel8080Emulator>(new Intel8080Emulator(memory));

    std::shared_ptr<ISmc5027Emulator> smc5027Emulator =
        std::shared_ptr<ISmc5027Emulator>(new Smc5027Emulator(logger));

    std::shared_ptr<IKeyboardEmulator> keyboardEmulator =
        std::shared_ptr<IKeyboardEmulator>(new KeyboardEmulator(logger));

    std::shared_ptr<IFloppyEmulator> floppy1Emulator =
        std::shared_ptr<IFloppyEmulator>(new FloppyEmulator());

    std::shared_ptr<IFloppyEmulator> floppy2Emulator =
        std::shared_ptr<IFloppyEmulator>(new FloppyEmulator());

    std::shared_ptr<ITms5501Emulator> tms5501Emulator =
        std::shared_ptr<ITms5501Emulator>(
            new Tms5501Emulator(
                logger,
                intel8080Emulator,
                keyboardEmulator,
                floppy1Emulator,
                floppy2Emulator
            )
        );

    std::shared_ptr<ICrtEmulator> crtEmulator =
        std::shared_ptr<ICrtEmulator>(
            new CrtEmulator(
                logger,
                memory,
                smc5027Emulator
            )
        );

    return std::unique_ptr<ICompucolorEmulator>(
        new CompucolorEmulator(
            crtEmulator,
            intel8080Emulator,
            keyboardEmulator,
            memory,
            smc5027Emulator,
            tms5501Emulator
        )
    );
}