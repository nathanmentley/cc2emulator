#include "FloppyEmulator.h"

Compucolor::Impl::Floppy::FloppyEmulator::FloppyEmulator()
{
}

void Compucolor::Impl::Floppy::FloppyEmulator::Start()
{
}

void Compucolor::Impl::Floppy::FloppyEmulator::Stop()
{
}

void Compucolor::Impl::Floppy::FloppyEmulator::Reset()
{
}

uint8_t Compucolor::Impl::Floppy::FloppyEmulator::Read(uint8_t port)
{
    return 0;
}

void Compucolor::Impl::Floppy::FloppyEmulator::Write(uint8_t port, uint8_t data)
{
}

void Compucolor::Impl::Floppy::FloppyEmulator::Select(bool selected, bool write, uint8_t stepper)
{
}

std::shared_ptr<Compucolor::Floppy::IFloppyEmulator> Compucolor::Impl::Floppy::FloppyEmulator::Creator(
    Compucolor::Common::IPluginManager* manager
)
{
    return std::shared_ptr<Compucolor::Floppy::IFloppyEmulator>(
        new Compucolor::Impl::Floppy::FloppyEmulator()
    );
}
