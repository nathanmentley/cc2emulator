#include <Compucolor/App/Replay/ManualScheduler.h>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

void Compucolor::Impl::App::Replay::ManualScheduledTask::Stop()
{
    _stopRequested.store(true, std::memory_order_relaxed);
}

bool Compucolor::Impl::App::Replay::ManualScheduledTask::IsStopRequested() const
{
    return _stopRequested.load(std::memory_order_relaxed);
}

std::future<int> Compucolor::Impl::App::Replay::ManualScheduler::RunOnce(
    const uint64_t afterNanoseconds,
    std::function<int()> logic
)
{
    if (!logic)
    {
        throw std::invalid_argument("ManualScheduler requires task logic");
    }

    std::promise<int> promise;
    std::future<int> future = promise.get_future();

    auto task = std::make_shared<Task>();
    task->next = _now + afterNanoseconds;
    task->interval = 0;
    task->order = _nextOrder++;
    task->logic = std::move(logic);
    task->handle = std::make_shared<ManualScheduledTask>();
    task->promise = std::move(promise);

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasks.push_back(task);
    }

    return future;
}

std::shared_ptr<Compucolor::Scheduler::IScheduledTask>
Compucolor::Impl::App::Replay::ManualScheduler::SetupRecurringTask(
    const uint64_t everyNanoseconds,
    std::function<int()> logic
)
{
    if (everyNanoseconds == 0)
    {
        throw std::invalid_argument("Recurring task interval must be positive");
    }

    if (!logic)
    {
        throw std::invalid_argument("ManualScheduler requires task logic");
    }

    auto task = std::make_shared<Task>();
    task->interval = everyNanoseconds;
    task->next = _now + everyNanoseconds;
    task->order = _nextOrder++;
    task->logic = logic;
    task->handle = std::make_shared<ManualScheduledTask>();

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasks.push_back(task);
    }

    // Production schedulers invoke recurring logic immediately, then wait for
    // the first interval. Match that behavior so Start() has the same effect.
    (void)logic();

    return task->handle;
}

void Compucolor::Impl::App::Replay::ManualScheduler::Advance(const uint64_t nanoseconds)
{
    uint64_t target = 0;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        target = _now + nanoseconds;
    }

    while (true)
    {
        std::shared_ptr<Task> next;

        {
            std::lock_guard<std::mutex> lock(_mutex);

            for (const auto& task : _tasks)
            {
                if (!task->handle || task->handle->IsStopRequested())
                {
                    continue;
                }

                if (task->next > target)
                {
                    continue;
                }

                if (!next ||
                    task->next < next->next ||
                    (task->next == next->next && task->order < next->order))
                {
                    next = task;
                }
            }

            if (!next)
            {
                break;
            }

            _now = next->next;
        }

        // Task logic may call Now() through the injected clock, so the scheduler
        // mutex must not be held while it runs.
        int result = 0;

        try
        {
            result = next->logic();
        }
        catch (...)
        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (next->promise.has_value())
            {
                next->promise->set_exception(std::current_exception());
            }

            throw;
        }

        std::lock_guard<std::mutex> lock(_mutex);

        if (next->promise.has_value())
        {
            next->promise->set_value(result);
        }

        if (next->interval == 0)
        {
            next->next = std::numeric_limits<uint64_t>::max();
        }
        else
        {
            next->next += next->interval;
        }

        _tasks.erase(
            std::remove_if(
                _tasks.begin(),
                _tasks.end(),
                [](const auto& task) {
                    return !task->handle || task->handle->IsStopRequested();
                }
            ),
            _tasks.end()
        );
    }

    std::lock_guard<std::mutex> lock(_mutex);
    _now = target;
}

void Compucolor::Impl::App::Replay::ManualScheduler::Tick()
{
    Advance(Interval());
}

uint64_t Compucolor::Impl::App::Replay::ManualScheduler::Now() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _now;
}
