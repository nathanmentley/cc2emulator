#include <Compucolor/App/JsonlDebugSink.h>

#include <Compucolor/App/DebugDisassembler.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

Compucolor::Impl::App::JsonlDebugSink::JsonlDebugSink(
    std::string path,
    TraceFilterConfig filters
):
    _path(std::move(path)),
    _filters(std::move(filters)),
    _stream(_path, std::ios::out | std::ios::trunc | std::ios::binary)
{
    if (!_stream.is_open())
    {
        throw std::runtime_error("Unable to open debug JSONL file: " + _path);
    }

    _worker = std::thread([this] { WorkerLoop(); });
}

Compucolor::Impl::App::JsonlDebugSink::~JsonlDebugSink()
{
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _stopping = true;
    }

    _queueCv.notify_all();

    if (_worker.joinable())
    {
        _worker.join();
    }

    std::lock_guard<std::mutex> lock(_streamMutex);

    if (_stream.is_open())
    {
        _stream.flush();
        _stream.close();
    }
}

void Compucolor::Impl::App::JsonlDebugSink::Flush()
{
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        _queueCv.wait(lock, [this] {
            return _queue.empty() && _pending == 0;
        });
    }

    std::lock_guard<std::mutex> lock(_streamMutex);

    if (_stream.is_open())
    {
        _stream.flush();
    }
}

void Compucolor::Impl::App::JsonlDebugSink::WorkerLoop()
{
    while (true)
    {
        TraceRecord record;

        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _queueCv.wait(lock, [this] {
                return !_queue.empty() || _stopping;
            });

            if (_queue.empty() && _stopping)
            {
                return;
            }

            record = std::move(_queue.front());
            _queue.pop_front();
        }

        std::visit([this](const auto& value) { ProcessRecord(value); }, record);

        {
            std::lock_guard<std::mutex> lock(_queueMutex);
            --_pending;
        }

        _queueCv.notify_all();
    }
}

void Compucolor::Impl::App::JsonlDebugSink::SetEnabled(const bool enabled)
{
    _enabled.store(enabled, std::memory_order_relaxed);
}

bool Compucolor::Impl::App::JsonlDebugSink::IsEnabled() const
{
    return _enabled.load(std::memory_order_relaxed);
}

void Compucolor::Impl::App::JsonlDebugSink::SetCpuState(
    const Compucolor::Debug::CpuState& state
)
{
    if (IsEnabled())
    {
        Enqueue(state);
    }
}

void Compucolor::Impl::App::JsonlDebugSink::SetCurrentPc(const uint16_t pc)
{
    _currentPc.store(pc, std::memory_order_relaxed);
}

uint16_t Compucolor::Impl::App::JsonlDebugSink::GetCurrentPc() const
{
    return _currentPc.load(std::memory_order_relaxed);
}

void Compucolor::Impl::App::JsonlDebugSink::OnInstruction(
    const Compucolor::Debug::InstructionEvent& event
)
{
    if (IsEnabled())
    {
        Enqueue(event);
    }
}

void Compucolor::Impl::App::JsonlDebugSink::OnPort(
    const Compucolor::Debug::PortEvent& event
)
{
    if (!IsEnabled())
    {
        return;
    }

    const auto pc = event.pc != 0 ? event.pc : GetCurrentPc();
    Enqueue(ResolvedPortEvent{event, pc});
}

void Compucolor::Impl::App::JsonlDebugSink::SetTms5501State(
    const Compucolor::Debug::Tms5501State& state
)
{
    if (IsEnabled())
    {
        Enqueue(state);
    }
}

void Compucolor::Impl::App::JsonlDebugSink::SetSmc5027State(
    const Compucolor::Debug::Smc5027State& state
)
{
    if (IsEnabled())
    {
        Enqueue(state);
    }
}

void Compucolor::Impl::App::JsonlDebugSink::ProcessRecord(
    const Compucolor::Debug::CpuState& state
)
{
    if (!_filters.Allows("cpu", state.pc))
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

void Compucolor::Impl::App::JsonlDebugSink::ProcessRecord(
    const Compucolor::Debug::InstructionEvent& event
)
{
    std::string text;

    if (_filters.WantsText("instr"))
    {
        text = Compucolor::Impl::App::Disassemble8080(
            event.bytes.data(),
            event.before.pc
        ).text;
    }

    if (!_filters.Allows("instr", event.before.pc, 0, false, text))
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

void Compucolor::Impl::App::JsonlDebugSink::ProcessRecord(
    const ResolvedPortEvent& record
)
{
    const auto& event = record.event;

    if (!_filters.Allows("port", record.pc, event.port, event.isWrite))
    {
        return;
    }

    boost::json::object line;
    line["kind"] = "port";
    line["pc"] = Hex(record.pc, 4);
    line["port"] = Hex(event.port, 2);
    line["value"] = Hex(event.value, 2);
    line["isWrite"] = event.isWrite;

    WriteLine(std::move(line));
}

void Compucolor::Impl::App::JsonlDebugSink::ProcessRecord(
    const Compucolor::Debug::Tms5501State& state
)
{
    if (!_filters.Allows("tms5501"))
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

void Compucolor::Impl::App::JsonlDebugSink::ProcessRecord(
    const Compucolor::Debug::Smc5027State& state
)
{
    if (!_filters.Allows("smc5027"))
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

void Compucolor::Impl::App::JsonlDebugSink::WriteLine(boost::json::object line)
{
    std::lock_guard<std::mutex> lock(_streamMutex);

    if (!_stream.is_open())
    {
        return;
    }

    line["v"] = 1;
    line["seq"] = _nextSequence++;

    _stream << boost::json::serialize(line) << '\n';
}

boost::json::string Compucolor::Impl::App::JsonlDebugSink::Hex(
    const uint64_t value,
    const int width
)
{
    std::ostringstream stream;
    stream << "0x" << std::hex << std::uppercase << std::setfill('0')
           << std::setw(width) << value;

    return boost::json::string(stream.str());
}
