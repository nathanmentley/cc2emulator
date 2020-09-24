#include "Intel8080Emulator.h"

Intel8080Emulator::Intel8080Emulator(
    std::shared_ptr<IMemory> memory
):
    _cpu(std::unique_ptr<i8080>(new i8080())),
    _memory(memory),
    _isRunning(false)
{

}

void Intel8080Emulator::Start()
{
    i8080_init(_cpu.get());

    _cpu->read_byte = [](void* userdata, uint16_t addr) {
        return ((IMemory*)userdata)->GetByte(addr);
    };
    _cpu->write_byte = [](void* userdata, uint16_t addr, uint8_t data) {
        if (addr >= 0x6000)
            ((IMemory*)userdata)->SetByte(addr, data);
    };
    _cpu->port_in = [](void* userdata, uint8_t port) {
        return (uint8_t)0;
    };
    _cpu->port_out = [](void* userdata, uint8_t port, uint8_t data) {

    };
    _cpu->userdata = _memory.get();

    _isRunning = true;
    _thread = std::thread(
        [this]() {
            while(_isRunning) {
                //std::this_thread::sleep_for (std::chrono::milliseconds(20));
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