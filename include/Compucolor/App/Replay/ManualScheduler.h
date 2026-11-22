#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include <Compucolor/Scheduler/IScheduledTask.h>
#include <Compucolor/Scheduler/IScheduler.h>

namespace Compucolor::Impl::App::Replay
{
    class ManualScheduledTask final: public Compucolor::Scheduler::IScheduledTask {
        public:
            void Stop() override;
            void Join() override {}
            [[nodiscard]] bool IsStopRequested() const override;

        private:
            std::atomic<bool> _stopRequested{false};
    };

    // IScheduler whose clock advances only when the replay runner calls
    // Advance(). Recurring tasks run at their due simulated times, so a 1 ms
    // machine tick and a 10 ms CRT refresh stay in a fixed order.
    class ManualScheduler final: public Compucolor::Scheduler::IScheduler {
        public:
            std::future<int> RunOnce(
                uint64_t afterNanoseconds,
                std::function<int()> logic
            ) override;

            std::shared_ptr<Compucolor::Scheduler::IScheduledTask> SetupRecurringTask(
                uint64_t everyNanoseconds,
                std::function<int()> logic
            ) override;

            void Advance(uint64_t nanoseconds);
            void Tick();

            [[nodiscard]] static constexpr uint64_t Interval()
            {
                return 1'000'000;
            }

            [[nodiscard]] uint64_t Now() const;

        private:
            struct Task {
                uint64_t interval = 0;
                uint64_t next = 0;
                std::size_t order = 0;
                std::function<int()> logic;
                std::shared_ptr<ManualScheduledTask> handle;
                std::optional<std::promise<int>> promise;
            };

            mutable std::mutex _mutex;
            uint64_t _now = 0;
            std::size_t _nextOrder = 0;
            std::vector<std::shared_ptr<Task>> _tasks;
    };
}
