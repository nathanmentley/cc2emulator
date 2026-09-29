#include "Scheduler.h"
#include "ScheduledTask.h"

#include <utility>

Compucolor::Impl::Scheduler::Scheduler::Scheduler(
    std::shared_ptr<Logger::ILogger> logger
):
    _logger(std::move(logger)) {}

std::future<int> Compucolor::Impl::Scheduler::Scheduler::RunOnce(
    const uint64_t afterNanoseconds,
    const std::function<int()> logic
)
{
    return std::async(std::launch::async, [=, this] {
        std::this_thread::sleep_for(std::chrono::nanoseconds{afterNanoseconds});

        int result = logic();

        _logger->LogTrace(
            "%s Result: %d",
            "Scheduler",
            result
        );

        return result;
    });
}

std::shared_ptr<Compucolor::Scheduler::IScheduledTask> Compucolor::Impl::Scheduler::Scheduler::SetupRecurringTask(
    const uint64_t everyNanoseconds,
    const std::function<int()> logic
)
{
    _logger->LogTrace(
        "%s running logic every %d nanoseconds",
        "Scheduler",
        everyNanoseconds
    );

    const std::shared_ptr<ScheduledTask> task = std::make_shared<ScheduledTask>();

    task->Start(everyNanoseconds, logic);

    return task;
}

int64_t Compucolor::Impl::Scheduler::Scheduler::GetCurrentTimeInNanoseconds()
{
    return
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        )
            .count();
}

std::shared_ptr<Compucolor::Scheduler::IScheduler> Compucolor::Impl::Scheduler::Scheduler::Creator(
    Common::IPluginManager* manager
)
{
    return std::make_shared<Scheduler>(manager->GetLogger());
}
