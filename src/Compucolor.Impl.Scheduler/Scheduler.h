#pragma once

#include <Compucolor/Logger/ILogger.h>

#include <Compucolor/Scheduler/IScheduler.h>

namespace Compucolor::Impl::Scheduler
{
    class Scheduler: public Compucolor::Scheduler::IScheduler {
        public:
            Scheduler(
                std::shared_ptr<Compucolor::Logger::ILogger> logger
            );

            virtual std::future<int> RunOnce(uint64_t afterNanoseconds, std::function<int()> logic) override;

            virtual std::thread SetupReoccuringTask(uint64_t everyNanoseconds, std::function<int()> logic) override;

        private:
            std::shared_ptr<Compucolor::Logger::ILogger> _logger;

            int64_t GetCurrentTimeInNanoseconds();
    };
}