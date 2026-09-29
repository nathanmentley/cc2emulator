#pragma once

#include <chrono>
#include <functional>
#include <future>

#include <Compucolor/Common/IPlugin.h>
#include <Compucolor/Scheduler/IScheduledTask.h>

namespace Compucolor::Scheduler
{
    class IScheduler: public Common::IPlugin {
        public:
            virtual ~IScheduler() = default;

            virtual std::future<int> RunOnce(uint64_t afterNanoseconds, std::function<int()> logic) = 0;

            virtual std::shared_ptr<IScheduledTask> SetupRecurringTask(
                uint64_t everyNanoseconds,
                std::function<int()> logic
            ) = 0;
    };
}