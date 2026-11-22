#pragma once

#include <Compucolor/Scheduler/IScheduledTask.h>

#include <future>

namespace Compucolor::Impl::Scheduler
{
    // One worker thread that runs a callback immediately, then waits against a
    // steady-clock deadline. Long callbacks delay later runs rather than overlap.
    class ScheduledTask final : public Compucolor::Scheduler::IScheduledTask {
    public:
        ScheduledTask();

        // Stops and joins the worker if still running.
        ~ScheduledTask() override;

        // Starts the worker; interval is in nanoseconds.
        void Start(uint64_t everyNanoseconds, std::function<int()> logic);

        void Stop() override;

        void Join() override;

        bool IsStopRequested() const override;

    private:
        std::atomic<bool> _stopRequested{false};
        std::mutex _mutex;
        std::condition_variable _cv;
        std::thread _thread;
    };
}
