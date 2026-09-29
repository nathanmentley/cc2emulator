#pragma once

#include <Compucolor/Scheduler/IScheduledTask.h>

#include <future>

namespace Compucolor::Impl::Scheduler
{
    class ScheduledTask final : public Compucolor::Scheduler::IScheduledTask {
    public:
        ScheduledTask();

        ~ScheduledTask() override;

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
