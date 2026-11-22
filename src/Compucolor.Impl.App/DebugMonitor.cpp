#include <Compucolor/App/DebugMonitor.h>

#include <Compucolor/App/DebugDisassembler.h>

#include <algorithm>
#include <cctype>
#include <utility>

namespace
{
    std::string ToUpper(std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](const unsigned char character) {
                return static_cast<char>(std::toupper(character));
            }
        );

        return value;
    }
}

Compucolor::Impl::App::DebugMonitor::DebugMonitor(
    std::shared_ptr<Compucolor::Memory::IMemory> memory
):
    _memory(std::move(memory))
{
}

bool Compucolor::Impl::App::DebugMonitor::IsEnabled() const
{
    return _enabled.load(std::memory_order_relaxed);
}

void Compucolor::Impl::App::DebugMonitor::SetEnabled(const bool enabled)
{
    _enabled.store(enabled, std::memory_order_relaxed);
}

void Compucolor::Impl::App::DebugMonitor::SetCaptureEnabled(const bool enabled)
{
    _captureEnabled.store(enabled, std::memory_order_relaxed);
}

bool Compucolor::Impl::App::DebugMonitor::IsCaptureEnabled() const
{
    return _captureEnabled.load(std::memory_order_relaxed);
}

void Compucolor::Impl::App::DebugMonitor::SetCpuState(
    const Compucolor::Debug::CpuState& state
)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _cpuState = state;
}

void Compucolor::Impl::App::DebugMonitor::SetCurrentPc(const uint16_t pc)
{
    _currentPc.store(pc, std::memory_order_relaxed);
}

uint16_t Compucolor::Impl::App::DebugMonitor::GetCurrentPc() const
{
    return _currentPc.load(std::memory_order_relaxed);
}

void Compucolor::Impl::App::DebugMonitor::OnInstruction(
    const Compucolor::Debug::InstructionEvent& event
)
{
    if (!IsEnabled() || !IsCaptureEnabled())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(_mutex);

    if (!MatchesInstruction(event, _instructionFilter))
    {
        return;
    }

    if (_instructionTriggered)
    {
        return;
    }

    if (_instructionFilter.stopOnMatch)
    {
        _instructionTriggered = true;
    }

    Compucolor::Debug::InstructionEvent entry = event;
    entry.sequence = _nextSequence++;

    _instructions.push_back(std::move(entry));

    if (_instructions.size() > MaxEntries)
    {
        _instructions.pop_front();
    }
}

void Compucolor::Impl::App::DebugMonitor::OnPort(
    const Compucolor::Debug::PortEvent& event
)
{
    if (!IsEnabled() || !IsCaptureEnabled())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(_mutex);

    if (!MatchesPort(event, _portFilter))
    {
        return;
    }

    if (_portTriggered)
    {
        return;
    }

    if (_portFilter.stopOnMatch)
    {
        _portTriggered = true;
    }

    Compucolor::Debug::PortEvent entry = event;
    entry.sequence = _nextSequence++;

    _ports.push_back(std::move(entry));

    if (_ports.size() > MaxEntries)
    {
        _ports.pop_front();
    }
}

void Compucolor::Impl::App::DebugMonitor::SetInstructionFilter(
    const DebugFilter& filter
)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _instructionFilter = filter;
}

void Compucolor::Impl::App::DebugMonitor::SetPortFilter(const DebugFilter& filter)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _portFilter = filter;
}

Compucolor::Impl::App::DebugFilter
Compucolor::Impl::App::DebugMonitor::GetInstructionFilter() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _instructionFilter;
}

Compucolor::Impl::App::DebugFilter
Compucolor::Impl::App::DebugMonitor::GetPortFilter() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _portFilter;
}

bool Compucolor::Impl::App::DebugMonitor::IsInstructionTriggered() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _instructionTriggered;
}

bool Compucolor::Impl::App::DebugMonitor::IsPortTriggered() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _portTriggered;
}

void Compucolor::Impl::App::DebugMonitor::RearmTriggers()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _instructionTriggered = false;
    _portTriggered = false;
}

void Compucolor::Impl::App::DebugMonitor::SetTms5501State(
    const Compucolor::Debug::Tms5501State& state
)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _tms5501State = state;
}

void Compucolor::Impl::App::DebugMonitor::SetSmc5027State(
    const Compucolor::Debug::Smc5027State& state
)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _smc5027State = state;
}

Compucolor::Debug::CpuState Compucolor::Impl::App::DebugMonitor::GetCpuState() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _cpuState;
}

Compucolor::Debug::Tms5501State
Compucolor::Impl::App::DebugMonitor::GetTms5501State() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _tms5501State;
}

Compucolor::Debug::Smc5027State
Compucolor::Impl::App::DebugMonitor::GetSmc5027State() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _smc5027State;
}

std::vector<Compucolor::Debug::InstructionEvent>
Compucolor::Impl::App::DebugMonitor::GetInstructions() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return { _instructions.begin(), _instructions.end() };
}

std::vector<Compucolor::Debug::PortEvent>
Compucolor::Impl::App::DebugMonitor::GetPorts() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return { _ports.begin(), _ports.end() };
}

uint8_t Compucolor::Impl::App::DebugMonitor::ReadMemory(const uint16_t address) const
{
    return _memory ? _memory->GetByte(address) : 0;
}

void Compucolor::Impl::App::DebugMonitor::WriteMemory(
    const uint16_t address,
    const uint8_t value
) const
{
    if (_memory)
    {
        _memory->SetByte(address, value);
    }
}

void Compucolor::Impl::App::DebugMonitor::Clear()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _instructions.clear();
    _ports.clear();
    _instructionTriggered = false;
    _portTriggered = false;
    _nextSequence = 1;
}

bool Compucolor::Impl::App::DebugMonitor::MatchesInstruction(
    const Compucolor::Debug::InstructionEvent& event,
    const DebugFilter& filter
)
{
    if (!filter.enabled)
    {
        return true;
    }

    if (filter.filterByPc)
    {
        const uint16_t low = std::min(filter.pcFrom, filter.pcTo);
        const uint16_t high = std::max(filter.pcFrom, filter.pcTo);

        if (event.before.pc < low || event.before.pc > high)
        {
            return false;
        }
    }

    if (!filter.text.empty())
    {
        const auto instruction = Disassemble8080(event.bytes.data(), event.before.pc);

        if (ToUpper(instruction.text).find(ToUpper(filter.text)) == std::string::npos)
        {
            return false;
        }
    }

    return true;
}

bool Compucolor::Impl::App::DebugMonitor::MatchesPort(
    const Compucolor::Debug::PortEvent& event,
    const DebugFilter& filter
)
{
    if (!filter.enabled)
    {
        return true;
    }

    if (filter.filterByPort)
    {
        const uint8_t low = std::min(filter.portFrom, filter.portTo);
        const uint8_t high = std::max(filter.portFrom, filter.portTo);

        if (event.port < low || event.port > high)
        {
            return false;
        }
    }

    if (filter.direction == DebugPortDirection::Read && event.isWrite)
    {
        return false;
    }

    if (filter.direction == DebugPortDirection::Write && !event.isWrite)
    {
        return false;
    }

    return true;
}
