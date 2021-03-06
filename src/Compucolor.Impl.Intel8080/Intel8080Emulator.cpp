#include "Intel8080Emulator.h"

Compucolor::Impl::Intel8080::Intel8080Emulator::Intel8080Emulator(
    std::shared_ptr<Compucolor::Memory::IMemory> memory
):
    _context(
        std::unique_ptr<Compucolor::Impl::Intel8080::Intel8080EmulatorContext>(
            new Compucolor::Impl::Intel8080::Intel8080EmulatorContext()
        )
    ),
    _cpu(
        std::unique_ptr<Compucolor::Impl::Intel8080::i8080>(
            new Compucolor::Impl::Intel8080::i8080()
        )
    ),
    _memory(memory)
{
    _context->SetMemory(_memory);
}

void Compucolor::Impl::Intel8080::Intel8080Emulator::Start()
{
    Compucolor::Impl::Intel8080::i8080_init(_cpu.get());

    _cpu->read_byte = Intel8080Emulator::ReadByte;
    _cpu->write_byte = Intel8080Emulator::WriteByte;

    _cpu->port_in = Intel8080Emulator::ReadPort;
    _cpu->port_out = Intel8080Emulator::WritePort;

    _cpu->userdata = _context.get();
}

void Compucolor::Impl::Intel8080::Intel8080Emulator::Step()
{
    Compucolor::Impl::Intel8080::i8080_step(_cpu.get());
}

void Compucolor::Impl::Intel8080::Intel8080Emulator::Stop()
{
}

void Compucolor::Impl::Intel8080::Intel8080Emulator::Reset()
{
    Stop();

    //TODO:
    //reset cpu state

    Start();
}

void Compucolor::Impl::Intel8080::Intel8080Emulator::SetBus(std::shared_ptr<Compucolor::Intel8080::IIntel8080Bus> intel8080Bus)
{
    _context->SetBus(intel8080Bus);
}

void Compucolor::Impl::Intel8080::Intel8080Emulator::RegisterInterrupt(uint8_t opcode)
{
    Compucolor::Impl::Intel8080::i8080_interrupt(_cpu.get(), opcode);
}

std::shared_ptr<Compucolor::Intel8080::IIntel8080Emulator> Compucolor::Impl::Intel8080::Intel8080Emulator::Creator(
    Compucolor::Common::IPluginManager* manager
)
{
    return std::shared_ptr<Compucolor::Intel8080::IIntel8080Emulator>(
        new Compucolor::Impl::Intel8080::Intel8080Emulator(manager->GetMemory())
    );
}

uint8_t Compucolor::Impl::Intel8080::Intel8080Emulator::ReadByte(void* userdata, uint16_t addr)
{
    std::optional<std::shared_ptr<Compucolor::Memory::IMemory>> memory =
        ((Compucolor::Impl::Intel8080::Intel8080EmulatorContext*)userdata)->GetMemory();

    if (memory.has_value())
    {
        return memory.value()->GetByte(addr);
    }

    return (uint8_t)0x00;
}

void Compucolor::Impl::Intel8080::Intel8080Emulator::WriteByte(void* userdata, uint16_t addr, uint8_t data)
{
    std::optional<std::shared_ptr<Compucolor::Memory::IMemory>> memory =
        ((Compucolor::Impl::Intel8080::Intel8080EmulatorContext*)userdata)->GetMemory();

    if (memory.has_value())
    {
        //Only write above the 0-3fff rom.
        if (addr >= 0x4000)
        {
            memory.value()->SetByte(addr, data);
        }
    }
}

uint8_t Compucolor::Impl::Intel8080::Intel8080Emulator::ReadPort(void* userdata, uint8_t port)
{
    std::optional<std::shared_ptr<Compucolor::Intel8080::IIntel8080Bus>> bus =
        ((Compucolor::Impl::Intel8080::Intel8080EmulatorContext*)userdata)->GetBus();

    if (bus.has_value())
    {
        return bus.value()->Read(port);
    }

    return (uint8_t)0x00;
}

void Compucolor::Impl::Intel8080::Intel8080Emulator::WritePort(void* userdata, uint8_t port, uint8_t data)
{
    std::optional<std::shared_ptr<Compucolor::Intel8080::IIntel8080Bus>> bus =
        ((Compucolor::Impl::Intel8080::Intel8080EmulatorContext*)userdata)->GetBus();

    if (bus.has_value())
    {
        return bus.value()->Write(port, data);
    }
}