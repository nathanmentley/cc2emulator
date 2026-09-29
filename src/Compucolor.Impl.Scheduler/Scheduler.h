#pragma once

#include <Compucolor/Common/IPluginManager.h>
#include <Compucolor/Logger/ILogger.h>
#include <Compucolor/Scheduler/IScheduler.h>

namespace Compucolor::Impl::Scheduler
{
    class Scheduler: public Compucolor::Scheduler::IScheduler {
        public:
            Scheduler(
                std::shared_ptr<Logger::ILogger> logger
            );

            std::future<int> RunOnce(uint64_t afterNanoseconds, std::function<int()> logic) override;

            std::shared_ptr<Compucolor::Scheduler::IScheduledTask> SetupRecurringTask(
                uint64_t everyNanoseconds,
                std::function<int()> logic
            ) override;

            static std::shared_ptr<IScheduler> Creator(Common::IPluginManager* manager);
        private:
            std::shared_ptr<Logger::ILogger> _logger;

            static int64_t GetCurrentTimeInNanoseconds();
    };
}