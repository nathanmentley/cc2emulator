#include "Scheduler.h"

Compucolor::Impl::Scheduler::Scheduler::Scheduler(
    std::shared_ptr<Logger::ILogger> logger
):
    _logger(logger) {}

std::future<int> Compucolor::Impl::Scheduler::Scheduler::RunOnce(uint64_t afterNanoseconds, std::function<int()> logic)
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

std::thread Compucolor::Impl::Scheduler::Scheduler::SetupReoccuringTask(uint64_t everyNanoseconds, std::function<int()> logic)
{
    _logger->LogTrace(
        "%s running logic every %d nanoseconds",
        "Scheduler",
        everyNanoseconds
    );

    std::packaged_task<void()> task(
        [=] {
            while (true)
            {
                int64_t lastRunTime = GetCurrentTimeInNanoseconds();
                int64_t nextRunTime = lastRunTime + everyNanoseconds;
                logic();


                while(nextRunTime > GetCurrentTimeInNanoseconds());
            }
        }
    );

    return std::thread(std::move(task));
}

int64_t Compucolor::Impl::Scheduler::Scheduler::GetCurrentTimeInNanoseconds()
{
    return
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        )
            .count();
}