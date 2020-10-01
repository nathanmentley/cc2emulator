#include "get_emulator.h"

std::unique_ptr<ICompucolorEmulator> get_emulator()
{
    std::shared_ptr<IMemory> memory =
        std::shared_ptr<IMemory>(new ByteArrayMemory());

    std::shared_ptr<ICrtEmulator> crtEmulator =
        std::shared_ptr<ICrtEmulator>(new CrtEmulator(memory));

    std::shared_ptr<IIntel8080Emulator> intel8080Emulator =
        std::shared_ptr<IIntel8080Emulator>(new Intel8080Emulator(memory));

    std::shared_ptr<ISmc5027Emulator> smc5027Emulator =
        std::shared_ptr<ISmc5027Emulator>(new Smc5027Emulator());

    std::shared_ptr<IKeyboardEmulator> keyboardEmulator =
        std::shared_ptr<IKeyboardEmulator>(new KeyboardEmulator());

    std::shared_ptr<ITms5501Emulator> tms5501Emulator =
        std::shared_ptr<ITms5501Emulator>(
            new Tms5501Emulator(
                intel8080Emulator,
                keyboardEmulator
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