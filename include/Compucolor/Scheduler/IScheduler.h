#pragma once

#include <chrono>
#include <functional>
#include <future>

#include <Compucolor/Common/IPlugin.h>
#include <Compucolor/Scheduler/IScheduledTask.h>

namespace Compucolor::Scheduler
{
    // Thread-pool-like service for delayed and recurring work.
    class IScheduler: public Common::IPlugin {
        public:
            virtual ~IScheduler() = default;

            // Waits, then runs logic on an async thread; the future carries its result.
            virtual std::future<int> RunOnce(uint64_t afterNanoseconds, std::function<int()> logic) = 0;

            // Starts a task that runs logic immediately and then every interval.
            // The returned task must be kept alive as long as it should run.
            virtual std::shared_ptr<IScheduledTask> SetupRecurringTask(
                uint64_t everyNanoseconds,
                std::function<int()> logic
            ) = 0;
    };
}
