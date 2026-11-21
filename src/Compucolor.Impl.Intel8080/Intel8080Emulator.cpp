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

uint64_t Compucolor::Impl::Intel8080::Intel8080Emulator::Step()
{
    const unsigned long previousCycles = _cpu->cyc;
    Compucolor::Impl::Intel8080::i8080_step(_cpu.get());
    const unsigned long elapsedCycles = _cpu->cyc - previousCycles;
    return elapsedCycles ? elapsedCycles : 4;
}

void Compucolor::Impl::Intel8080::Intel8080Emulator::Stop()
{
}

std::unique_ptr<boost::json::value>
Compucolor::Impl::Intel8080::Intel8080Emulator::SaveState()
{
    boost::json::object state;

    state["cyc"] = static_cast<uint64_t>(_cpu->cyc);

    state["pc"] = static_cast<uint64_t>(_cpu->pc);
    state["sp"] = static_cast<uint64_t>(_cpu->sp);

    state["a"] = static_cast<uint64_t>(_cpu->a);
    state["b"] = static_cast<uint64_t>(_cpu->b);
    state["c"] = static_cast<uint64_t>(_cpu->c);
    state["d"] = static_cast<uint64_t>(_cpu->d);
    state["e"] = static_cast<uint64_t>(_cpu->e);
    state["h"] = static_cast<uint64_t>(_cpu->h);
    state["l"] = static_cast<uint64_t>(_cpu->l);

    state["sf"] = _cpu->sf;
    state["zf"] = _cpu->zf;
    state["hf"] = _cpu->hf;
    state["pf"] = _cpu->pf;
    state["cf"] = _cpu->cf;
    state["iff"] = _cpu->iff;

    state["halted"] = _cpu->halted;

    state["interruptPending"] = _cpu->interrupt_pending;
    state["interruptVector"] = static_cast<uint64_t>(_cpu->interrupt_vector);
    state["interruptDelay"] = static_cast<uint64_t>(_cpu->interrupt_delay);

    return std::make_unique<boost::json::value>(std::move(state));
}

void Compucolor::Impl::Intel8080::Intel8080Emulator::LoadState(
    std::shared_ptr<boost::json::value> value
)
{
    if (!value || !value->is_object())
    {
        throw std::runtime_error("Invalid Intel8080 state");
    }

    const auto& state = value->as_object();

    auto getUnsigned = [](const boost::json::value& value) -> uint64_t
    {
        if (value.is_uint64())
        {
            return value.as_uint64();
        }

        if (value.is_int64() && value.as_int64() >= 0)
        {
            return static_cast<uint64_t>(value.as_int64());
        }

        throw std::runtime_error(
            "Expected non-negative integer in Intel8080 state"
        );
    };

    auto getByte = [&getUnsigned](const boost::json::value& value) -> uint8_t
    {
        const uint64_t result = getUnsigned(value);

        if (result > UINT8_MAX)
        {
            throw std::runtime_error(
                "Invalid byte in Intel8080 state"
            );
        }

        return static_cast<uint8_t>(result);
    };

    auto getWord = [&getUnsigned](const boost::json::value& value) -> uint16_t
    {
        const uint64_t result = getUnsigned(value);

        if (result > UINT16_MAX)
        {
            throw std::runtime_error(
                "Invalid word in Intel8080 state"
            );
        }

        return static_cast<uint16_t>(result);
    };

    // Validate and stage all values before modifying the CPU.

    const uint64_t cyc = getUnsigned(state.at("cyc"));

    if (cyc > std::numeric_limits<unsigned long>::max())
    {
        throw std::runtime_error(
            "Intel8080 cycle count exceeds platform limits"
        );
    }

    const uint16_t pc = getWord(state.at("pc"));
    const uint16_t sp = getWord(state.at("sp"));

    const uint8_t a = getByte(state.at("a"));
    const uint8_t b = getByte(state.at("b"));
    const uint8_t c = getByte(state.at("c"));
    const uint8_t d = getByte(state.at("d"));
    const uint8_t e = getByte(state.at("e"));
    const uint8_t h = getByte(state.at("h"));
    const uint8_t l = getByte(state.at("l"));

    const bool sf = state.at("sf").as_bool();
    const bool zf = state.at("zf").as_bool();
    const bool hf = state.at("hf").as_bool();
    const bool pf = state.at("pf").as_bool();
    const bool cf = state.at("cf").as_bool();
    const bool iff = state.at("iff").as_bool();

    const bool halted = state.at("halted").as_bool();

    const bool interruptPending =
        state.at("interruptPending").as_bool();

    const uint8_t interruptVector =
        getByte(state.at("interruptVector"));

    const uint8_t interruptDelay =
        getByte(state.at("interruptDelay"));

    // Apply the restored state.

    _cpu->cyc = static_cast<unsigned long>(cyc);

    _cpu->pc = pc;
    _cpu->sp = sp;

    _cpu->a = a;
    _cpu->b = b;
    _cpu->c = c;
    _cpu->d = d;
    _cpu->e = e;
    _cpu->h = h;
    _cpu->l = l;

    _cpu->sf = sf;
    _cpu->zf = zf;
    _cpu->hf = hf;
    _cpu->pf = pf;
    _cpu->cf = cf;
    _cpu->iff = iff;

    _cpu->halted = halted;

    _cpu->interrupt_pending = interruptPending;
    _cpu->interrupt_vector = interruptVector;
    _cpu->interrupt_delay = interruptDelay;
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

bool Compucolor::Impl::Intel8080::Intel8080Emulator::IsInterruptPending() {
    return i8080_is_interrupt_pending(_cpu.get());
}

uint8_t Compucolor::Impl::Intel8080::Intel8080Emulator::GetPendingInterrupt() {
    return _cpu->interrupt_vector;
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