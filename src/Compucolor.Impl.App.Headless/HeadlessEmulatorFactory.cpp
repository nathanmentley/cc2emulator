#include <Compucolor/App/Replay/HeadlessEmulatorFactory.h>

#include <Compucolor/Logger/ConsoleLoggerProvider.h>

#include "CompucolorEmulator.h"
#include "CrtEmulator.h"
#include "FloppyEmulator.h"
#include "Intel8080Emulator.h"
#include "KeyboardEmulator.h"
#include "Logger.h"
#include "Smc5027Emulator.h"
#include "Tms5501Emulator.h"
#include "VectorMemory.h"

namespace
{
    class NullDisplay final: public Compucolor::Common::IDisplay {
        public:
            void DrawPixel(Compucolor::Common::Color, int, int) override {}
            void Repaint() override {}
            void Close() override {}
    };

    class NullAudio final: public Compucolor::Common::IAudioOutput {
        public:
            void SetLevel(bool, uint64_t) override {}
            void Advance(uint64_t) override {}
    };
}

Compucolor::Impl::App::Replay::HeadlessMachine
Compucolor::Impl::App::Replay::CreateHeadlessMachine()
{
    auto scheduler = std::make_shared<ManualScheduler>();

    auto provider = std::make_shared<Compucolor::Logger::ConsoleLoggerProvider>(
        Compucolor::Logger::LogLevel::Error
    );
    auto logger = std::make_shared<Compucolor::Impl::Logger::Logger>(provider);

    auto memory = std::make_shared<Compucolor::Impl::Memory::VectorMemory>();
    auto intel8080 =
        std::make_shared<Compucolor::Impl::Intel8080::Intel8080Emulator>(memory);
    auto keyboard =
        std::make_shared<Compucolor::Impl::Keyboard::KeyboardEmulator>(logger);
    auto floppyA =
        std::make_shared<Compucolor::Impl::Floppy::FloppyEmulator>(logger);
    auto floppyB =
        std::make_shared<Compucolor::Impl::Floppy::FloppyEmulator>(logger);
    auto audio = std::make_shared<NullAudio>();
    auto tms5501 = std::make_shared<Compucolor::Impl::Tms5501::Tms5501Emulator>(
        logger,
        intel8080,
        keyboard,
        floppyA,
        floppyB,
        audio
    );
    auto smc5027 =
        std::make_shared<Compucolor::Impl::Smc5027::Smc5027Emulator>(logger);
    auto display = std::make_shared<NullDisplay>();
    auto crt = std::make_shared<Compucolor::Impl::Crt::CrtEmulator>(
        logger,
        display,
        scheduler,
        memory,
        smc5027,
        tms5501
    );

    auto emulator = std::make_shared<Compucolor::Impl::CompucolorII::CompucolorEmulator>(
        crt,
        floppyA,
        floppyB,
        intel8080,
        keyboard,
        memory,
        scheduler,
        smc5027,
        tms5501,
        [scheduler] { return scheduler->Now(); }
    );

    return { emulator, scheduler };
}
