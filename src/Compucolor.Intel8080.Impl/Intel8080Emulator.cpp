#include "Intel8080Emulator.h"

Intel8080Emulator::Intel8080Emulator(
    std::shared_ptr<IMemory> memory
):
    _context(std::unique_ptr<Intel8080EmulatorContext>(new Intel8080EmulatorContext())),
    _cpu(std::unique_ptr<i8080>(new i8080())),
    _memory(memory),
    _isRunning(false)
{
    _context->SetMemory(_memory);
}

void Intel8080Emulator::Start()
{
    i8080_init(_cpu.get());

    _cpu->read_byte = [](void* userdata, uint16_t addr) {
        std::optional<std::shared_ptr<IMemory>> memory = ((Intel8080EmulatorContext*)userdata)->GetMemory();

        if (memory.has_value())
        {
            return memory.value()->GetByte(addr);
        }

        return (uint8_t)0x00;
    };

    _cpu->write_byte = [](void* userdata, uint16_t addr, uint8_t data) {
        std::optional<std::shared_ptr<IMemory>> memory = ((Intel8080EmulatorContext*)userdata)->GetMemory();

        if (memory.has_value())
        {
            if (addr >= 0x6000)
            {
                memory.value()->SetByte(addr, data);
            }
        }

    };

    _cpu->port_in = [](void* userdata, uint8_t port) {
        std::optional<std::shared_ptr<IIntel8080Bus>> bus = ((Intel8080EmulatorContext*)userdata)->GetBus();

        if (bus.has_value())
        {
            return bus.value()->Read(port);
        }

        return (uint8_t)0x00;
    };

    _cpu->port_out = [](void* userdata, uint8_t port, uint8_t data) {
        std::optional<std::shared_ptr<IIntel8080Bus>> bus = ((Intel8080EmulatorContext*)userdata)->GetBus();

        if (bus.has_value())
        {
            return bus.value()->Write(port, data);
        }
    };

    _cpu->userdata = _context.get();

    _isRunning = true;
    _thread = std::thread(
        [this]() {
            while(_isRunning) {
                i8080_step(_cpu.get());
                //i8080_debug_output(_cpu.get(), false);
            }
        }
    );
}

void Intel8080Emulator::Stop()
{
    _isRunning = false;
    _thread.join();
}

void Intel8080Emulator::SetBus(std::shared_ptr<IIntel8080Bus> intel8080Bus)
{
    _context->SetBus(intel8080Bus);
}