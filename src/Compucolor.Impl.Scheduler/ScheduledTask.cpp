#include "ScheduledTask.h"

Compucolor::Impl::Scheduler::ScheduledTask::ScheduledTask() = default;

Compucolor::Impl::Scheduler::ScheduledTask::~ScheduledTask()
{
    Stop();
    Join();
}

void Compucolor::Impl::Scheduler::ScheduledTask::Start(uint64_t everyNanoseconds, std::function<int()> logic)
{
    _thread = std::thread([this, everyNanoseconds, logic = std::move(logic)]() mutable {
        auto nextRunTime = std::chrono::steady_clock::now();

        while (!_stopRequested.load(std::memory_order_relaxed))
        {
            nextRunTime += std::chrono::nanoseconds(everyNanoseconds);

            logic();

            std::unique_lock lock(_mutex);

            _cv.wait_until(lock, nextRunTime, [this] {
                return _stopRequested.load(std::memory_order_relaxed);
            });
        }
    });
}

void Compucolor::Impl::Scheduler::ScheduledTask::Stop()
{
    const bool was = _stopRequested.exchange(true, std::memory_order_relaxed);

    if (!was)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _cv.notify_all();
    }
}

void Compucolor::Impl::Scheduler::ScheduledTask::Join()
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

bool Compucolor::Impl::Scheduler::ScheduledTask::IsStopRequested() const
{
    return _stopRequested.load(std::memory_order_relaxed);
}