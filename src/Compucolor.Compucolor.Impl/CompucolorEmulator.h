
#pragma once

#include <memory>

#include <fruit/fruit.h>

#include "Compucolor.Core/IMemory.h"
#include "Compucolor.Compucolor/ICompucolorEmulator.h"
#include "Compucolor.Crt/ICrtEmulator.h"
#include "Compucolor.Intel8080/IIntel8080Emulator.h"

#include "Roms.h"

class CompucolorEmulator: public ICompucolorEmulator {
    public:
        INJECT(
            CompucolorEmulator(
                std::shared_ptr<IMemory> memory,
                std::shared_ptr<ICrtEmulator> crt,
                std::shared_ptr<IIntel8080Emulator> intel8080
            )
        );

        virtual void Start() override;
        virtual void Stop() override;

        virtual void SetDisplay(IDisplay* display) override;

    private:
        std::shared_ptr<IMemory> _memory;
        std::shared_ptr<ICrtEmulator> _crt;
        std::shared_ptr<IIntel8080Emulator> _intel8080;
};