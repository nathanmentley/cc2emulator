#include <Compucolor.Scheduler.Impl/Scheduler.h>

Scheduler::Scheduler(
    std::shared_ptr<ILogger> logger
):
    _logger(logger) {}

std::future<int> Scheduler::RunOnce(uint64_t afterNanoseconds, std::function<int()> logic)
{
    return std::async(std::launch::async, [=] {
        std::this_thread::sleep_for(std::chrono::nanoseconds{afterNanoseconds});

        int result = logic();

        _logger->LogTrace(
            "%s Result: %d",
            NAMEOF_TYPE(Scheduler),
            result
        );

        return result;
    });
}

std::thread Scheduler::SetupReoccuringTask(uint64_t everyNanoseconds, std::function<int()> logic)
{
    _logger->LogTrace(
        "%s running logic every %d nanoseconds",
        NAMEOF_TYPE(Scheduler),
        everyNanoseconds
    );

    std::packaged_task<void()> task(
        [=] {
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::nanoseconds{everyNanoseconds});

                logic();
            }
        }
    );

    return std::thread(std::move(task));
}