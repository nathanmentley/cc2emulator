#include "DebugHub.h"

#include <utility>

void Compucolor::Impl::App::Sfml::DebugHub::AddSink(
    std::shared_ptr<Compucolor::Debug::IDebugSink> sink
)
{
    if (!sink)
    {
        return;
    }

    const bool enabled = IsEnabled();

    {
        std::lock_guard<std::mutex> lock(_sinkMutex);
        _sinks.push_back(sink);
    }

    if (const auto recordable = std::dynamic_pointer_cast<IRecordableDebugSink>(sink))
    {
        recordable->SetEnabled(enabled);
    }
}

void Compucolor::Impl::App::Sfml::DebugHub::SetEnabled(const bool enabled)
{
    _enabled.store(enabled, std::memory_order_relaxed);

    for (const auto& sink : CopySinks())
    {
        if (const auto recordable = std::dynamic_pointer_cast<IRecordableDebugSink>(sink))
        {
            recordable->SetEnabled(enabled);
        }
    }
}

bool Compucolor::Impl::App::Sfml::DebugHub::IsEnabled() const
{
    return _enabled.load(std::memory_order_relaxed);
}

void Compucolor::Impl::App::Sfml::DebugHub::SetCpuState(
    const Compucolor::Debug::CpuState& state
)
{
    for (const auto& sink : CopySinks())
    {
        sink->SetCpuState(state);
    }
}

void Compucolor::Impl::App::Sfml::DebugHub::SetCurrentPc(const uint16_t pc)
{
    _currentPc.store(pc, std::memory_order_relaxed);

    for (const auto& sink : CopySinks())
    {
        sink->SetCurrentPc(pc);
    }
}

uint16_t Compucolor::Impl::App::Sfml::DebugHub::GetCurrentPc() const
{
    return _currentPc.load(std::memory_order_relaxed);
}

void Compucolor::Impl::App::Sfml::DebugHub::OnInstruction(
    const Compucolor::Debug::InstructionEvent& event
)
{
    if (!IsEnabled())
    {
        return;
    }

    for (const auto& sink : CopySinks())
    {
        sink->OnInstruction(event);
    }
}

void Compucolor::Impl::App::Sfml::DebugHub::OnPort(
    const Compucolor::Debug::PortEvent& event
)
{
    if (!IsEnabled())
    {
        return;
    }

    for (const auto& sink : CopySinks())
    {
        sink->OnPort(event);
    }
}

void Compucolor::Impl::App::Sfml::DebugHub::SetTms5501State(
    const Compucolor::Debug::Tms5501State& state
)
{
    for (const auto& sink : CopySinks())
    {
        sink->SetTms5501State(state);
    }
}

void Compucolor::Impl::App::Sfml::DebugHub::SetSmc5027State(
    const Compucolor::Debug::Smc5027State& state
)
{
    for (const auto& sink : CopySinks())
    {
        sink->SetSmc5027State(state);
    }
}

std::vector<std::shared_ptr<Compucolor::Debug::IDebugSink>>
Compucolor::Impl::App::Sfml::DebugHub::CopySinks() const
{
    std::lock_guard<std::mutex> lock(_sinkMutex);
    return _sinks;
}
