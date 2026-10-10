#pragma once

#include <functional>
#include <future>
#include <memory>

#include <Compucolor/Scheduler/IScheduler.h>

class FakeScheduledTask: public Compucolor::Scheduler::IScheduledTask
{
    public:
        void Stop() override { _stopRequested = true; }
        void Join() override {}
        [[nodiscard]] bool IsStopRequested() const override { return _stopRequested; }

    private:
        bool _stopRequested = false;
};

// Runs recurring work only when the test calls Tick, or once at setup if asked to.
class FakeScheduler: public Compucolor::Scheduler::IScheduler
{
    public:
        explicit FakeScheduler(bool runOnSetup = false): _runOnSetup(runOnSetup) {}

        std::future<int> RunOnce(uint64_t, std::function<int()> logic) override
        {
            std::promise<int> promise;
            promise.set_value(logic());

            return promise.get_future();
        }

        std::shared_ptr<Compucolor::Scheduler::IScheduledTask> SetupRecurringTask(
            uint64_t everyNanoseconds,
            std::function<int()> logic
        ) override
        {
            interval = everyNanoseconds;
            _logic = std::move(logic);

            if (_runOnSetup)
            {
                _logic();
            }

            return std::make_shared<FakeScheduledTask>();
        }

        void Tick()
        {
            _logic();
        }

        uint64_t interval = 0;

    private:
        bool _runOnSetup;
        std::function<int()> _logic;
};
