#pragma once

#include <Compucolor/Common/IPluginManager.h>
#include <Compucolor/Logger/ILogger.h>
#include <Compucolor/Scheduler/IScheduler.h>

namespace Compucolor::Impl::Scheduler
{
    // Creates delayed one-shot tasks and recurring tasks backed by worker threads.
    class Scheduler: public Compucolor::Scheduler::IScheduler {
        public:
            Scheduler(
                std::shared_ptr<Logger::ILogger> logger
            );

            // Sleeps for the delay on an async thread, then runs logic and returns its result.
            std::future<int> RunOnce(uint64_t afterNanoseconds, std::function<int()> logic) override;

            // Starts a task immediately; the returned handle owns its worker thread.
            std::shared_ptr<Compucolor::Scheduler::IScheduledTask> SetupRecurringTask(
                uint64_t everyNanoseconds,
                std::function<int()> logic
            ) override;

            static std::shared_ptr<IScheduler> Creator(Common::IPluginManager* manager);
        private:
            std::shared_ptr<Logger::ILogger> _logger;

            // Helper for absolute steady-clock timestamps (currently unused).
            static int64_t GetCurrentTimeInNanoseconds();
    };
}