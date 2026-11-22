#include "JsonlDebugSink.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

Compucolor::Impl::App::Sfml::JsonlDebugSink::JsonlDebugSink(std::string path):
    _path(std::move(path)),
    _stream(_path, std::ios::out | std::ios::trunc | std::ios::binary)
{
    if (!_stream.is_open())
    {
        throw std::runtime_error("Unable to open debug JSONL file: " + _path);
    }
}

Compucolor::Impl::App::Sfml::JsonlDebugSink::~JsonlDebugSink()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_stream.is_open())
    {
        _stream.flush();
        _stream.close();
    }
}

void Compucolor::Impl::App::Sfml::JsonlDebugSink::SetEnabled(const bool enabled)
{
    _enabled.store(enabled, std::memory_order_relaxed);
}

bool Compucolor::Impl::App::Sfml::JsonlDebugSink::IsEnabled() const
{
    return _enabled.load(std::memory_order_relaxed);
}

void Compucolor::Impl::App::Sfml::JsonlDebugSink::SetCpuState(
    const Compucolor::Debug::CpuState& state
)
{
    if (!IsEnabled())
    {
        return;
    }

    boost::json::object line;
    line["kind"] = "cpu";
    line["pc"] = Hex(state.pc, 4);
    line["sp"] = Hex(state.sp, 4);
    line["a"] = Hex(state.a, 2);
    line["b"] = Hex(state.b, 2);
    line["c"] = Hex(state.c, 2);
    line["d"] = Hex(state.d, 2);
    line["e"] = Hex(state.e, 2);
    line["h"] = Hex(state.h, 2);
    line["l"] = Hex(state.l, 2);
    line["cycles"] = state.cycles;
    line["emulatedNanoseconds"] = state.emulatedNanoseconds;
    line["interruptVector"] = Hex(state.interruptVector, 2);
    line["interruptDelay"] = state.interruptDelay;

    boost::json::object flags;
    flags["sf"] = state.sf;
    flags["zf"] = state.zf;
    flags["hf"] = state.hf;
    flags["pf"] = state.pf;
    flags["cf"] = state.cf;
    flags["iff"] = state.iff;
    flags["halted"] = state.halted;
    flags["interruptPending"] = state.interruptPending;
    line["flags"] = std::move(flags);

    WriteLine(std::move(line));
}

void Compucolor::Impl::App::Sfml::JsonlDebugSink::SetCurrentPc(const uint16_t pc)
{
    _currentPc.store(pc, std::memory_order_relaxed);
}

uint16_t Compucolor::Impl::App::Sfml::JsonlDebugSink::GetCurrentPc() const
{
    return _currentPc.load(std::memory_order_relaxed);
}

void Compucolor::Impl::App::Sfml::JsonlDebugSink::OnInstruction(
    const Compucolor::Debug::InstructionEvent& event
)
{
    if (!IsEnabled())
    {
        return;
    }

    boost::json::object line;
    line["kind"] = "instr";
    line["pc"] = Hex(event.before.pc, 4);
    line["cycles"] = event.cycles;

    boost::json::array bytes;

    for (const auto byte : event.bytes)
    {
        bytes.push_back(Hex(byte, 2));
    }

    line["bytes"] = std::move(bytes);
    WriteLine(std::move(line));
}

void Compucolor::Impl::App::Sfml::JsonlDebugSink::OnPort(
    const Compucolor::Debug::PortEvent& event
)
{
    if (!IsEnabled())
    {
        return;
    }

    const auto pc = event.pc != 0 ? event.pc : GetCurrentPc();

    boost::json::object line;
    line["kind"] = "port";
    line["pc"] = Hex(pc, 4);
    line["port"] = Hex(event.port, 2);
    line["value"] = Hex(event.value, 2);
    line["isWrite"] = event.isWrite;

    WriteLine(std::move(line));
}

void Compucolor::Impl::App::Sfml::JsonlDebugSink::SetTms5501State(
    const Compucolor::Debug::Tms5501State& state
)
{
    if (!IsEnabled())
    {
        return;
    }

    boost::json::object line;
    line["kind"] = "tms5501";
    line["status"] = Hex(state.status, 2);
    line["intMask"] = Hex(state.intMask, 2);
    line["intStatus"] = Hex(state.intStatus, 2);
    line["outport"] = Hex(state.outport, 2);
    line["rate"] = Hex(state.rate, 2);
    line["dscCmd"] = Hex(state.dscCmd, 2);
    line["rxData"] = Hex(state.rxData, 2);
    line["txData"] = Hex(state.txData, 2);
    line["txDataCount"] = state.txDataCount;
    line["pendingInterrupt"] = Hex(state.pendingInterrupt, 2);
    line["audioLevel"] = state.audioLevel;
    line["serialSelected"] = state.serialSelected;
    line["floppy1Selected"] = state.floppy1Selected;
    line["floppy2Selected"] = state.floppy2Selected;

    boost::json::array timers;

    for (const auto timer : state.timers)
    {
        timers.push_back(Hex(timer, 2));
    }

    line["timers"] = std::move(timers);

    WriteLine(std::move(line));
}

void Compucolor::Impl::App::Sfml::JsonlDebugSink::SetSmc5027State(
    const Compucolor::Debug::Smc5027State& state
)
{
    if (!IsEnabled())
    {
        return;
    }

    boost::json::object line;
    line["kind"] = "smc5027";

    boost::json::array cursor;
    cursor.push_back(Hex(state.cursorX, 2));
    cursor.push_back(Hex(state.cursorY, 2));
    line["cursor"] = std::move(cursor);

    line["cursorX"] = Hex(state.cursorX, 2);
    line["cursorY"] = Hex(state.cursorY, 2);
    line["firstDisplayRow"] = Hex(state.firstDisplayRow, 2);

    WriteLine(std::move(line));
}

void Compucolor::Impl::App::Sfml::JsonlDebugSink::WriteLine(boost::json::object line)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_stream.is_open())
    {
        return;
    }

    line["v"] = 1;
    line["seq"] = _nextSequence++;

    _stream << boost::json::serialize(line) << '\n';
}

boost::json::string Compucolor::Impl::App::Sfml::JsonlDebugSink::Hex(
    const uint64_t value,
    const int width
)
{
    std::ostringstream stream;
    stream << "0x" << std::hex << std::uppercase << std::setfill('0')
           << std::setw(width) << value;

    return boost::json::string(stream.str());
}
