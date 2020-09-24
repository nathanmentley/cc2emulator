#include "get_emulator.h"

fruit::Component<ICompucolorEmulator> getEmulator() {
    return fruit::createComponent()
        .bind<IMemory, ByteArrayMemory>()
        .bind<ICrtEmulator, CrtEmulator>()
        .bind<IIntel8080Emulator, Intel8080Emulator>()
        .bind<ICompucolorEmulator, CompucolorEmulator>();
}

std::unique_ptr<ICompucolorEmulator> get_emulator()
{
    fruit::Injector<ICompucolorEmulator> injector(getEmulator);
    ICompucolorEmulator* instance = injector.get<ICompucolorEmulator*>();

    return std::unique_ptr<ICompucolorEmulator>(instance);
}