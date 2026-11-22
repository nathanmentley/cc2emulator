#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <Compucolor/App/Replay/ManualScheduler.h>

using Compucolor::Impl::App::Replay::ManualScheduler;

TEST(ManualSchedulerTests, RunsRecurringTasksAtSimulatedDueTimes)
{
    ManualScheduler scheduler;
    int machineTicks = 0;
    int crtRefreshes = 0;
    std::vector<std::string> order;

    scheduler.SetupRecurringTask(
        10'000'000,
        [&] {
            crtRefreshes++;
            order.emplace_back("crt");
            return 0;
        }
    );

    scheduler.SetupRecurringTask(
        1'000'000,
        [&] {
            machineTicks++;
            order.emplace_back("machine");
            return 0;
        }
    );

    // Both tasks run once at registration, matching the production scheduler.
    EXPECT_EQ(machineTicks, 1);
    EXPECT_EQ(crtRefreshes, 1);

    scheduler.Advance(10'000'000);

    EXPECT_EQ(machineTicks, 11);
    EXPECT_EQ(crtRefreshes, 2);
    EXPECT_EQ(scheduler.Now(), 10'000'000U);

    // The 10 ms boundary runs the CRT task before the machine task because the
    // CRT task was registered first.
    ASSERT_GE(order.size(), 13U);
    EXPECT_EQ(order[order.size() - 2], "crt");
    EXPECT_EQ(order[order.size() - 1], "machine");
}

TEST(ManualSchedulerTests, StopsRecurringTasks)
{
    ManualScheduler scheduler;
    int calls = 0;

    auto task = scheduler.SetupRecurringTask(
        1'000'000,
        [&] {
            calls++;
            return 0;
        }
    );

    task->Stop();
    scheduler.Advance(5'000'000);

    EXPECT_EQ(calls, 1);
}

TEST(ManualSchedulerTests, CompletesRunOnceAtItsDueTime)
{
    ManualScheduler scheduler;
    auto future = scheduler.RunOnce(
        2'000'000,
        [] { return 42; }
    );

    scheduler.Advance(1'000'000);
    EXPECT_NE(future.wait_for(std::chrono::seconds(0)), std::future_status::ready);

    scheduler.Advance(1'000'000);
    ASSERT_EQ(future.wait_for(std::chrono::seconds(0)), std::future_status::ready);
    EXPECT_EQ(future.get(), 42);
}
