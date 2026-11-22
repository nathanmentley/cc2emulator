#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <vector>

#include <boost/json.hpp>

#include <Compucolor/App/Replay/ReplayBundle.h>
#include <Compucolor/App/Replay/ReplayInputSink.h>

using Compucolor::Debug::CpuState;
using Compucolor::Keyboard::CompucolorIIKey;
using Compucolor::Impl::App::Replay::ReplayBundle;
using Compucolor::Impl::App::Replay::ReplayEvent;
using Compucolor::Impl::App::Replay::ReplayInputSink;

namespace
{
    ReplayBundle TestBundle()
    {
        boost::json::object state;
        state["emulatedNanoseconds"] = std::uint64_t{100};

        const std::vector<ReplayEvent> input{
            {150, CompucolorIIKey::A, true},
            {250, CompucolorIIKey::A, false}
        };

        return ReplayBundle(state, input, 300, "test");
    }

    CpuState CpuAt(const uint64_t nanoseconds)
    {
        CpuState cpu{};
        cpu.emulatedNanoseconds = nanoseconds;
        return cpu;
    }
}

TEST(ReplayInputSinkTests, AppliesEventsAtCpuSnapshotTimes)
{
    const auto bundle = TestBundle();
    std::vector<ReplayEvent> applied;

    ReplayInputSink sink;
    sink.Start(bundle, [&applied](const ReplayEvent& event) {
        applied.push_back(event);
    });

    ASSERT_TRUE(sink.IsActive());

    sink.SetCpuState(CpuAt(200));
    sink.OnTickBoundary(200);

    ASSERT_EQ(applied.size(), 1U);
    EXPECT_TRUE(applied[0].down);
    EXPECT_EQ(applied[0].emulatedNanoseconds, 150U);

    const auto progress = sink.GetProgress();
    EXPECT_TRUE(progress.active);
    EXPECT_EQ(progress.currentNanoseconds, 200U);
    EXPECT_EQ(progress.nextEventNanoseconds, 250U);
    EXPECT_EQ(progress.stopNanoseconds, 300U);
    EXPECT_EQ(progress.appliedEvents, 1U);
    EXPECT_EQ(progress.totalEvents, 2U);

    sink.SetCpuState(CpuAt(300));
    sink.OnTickBoundary(300);

    ASSERT_EQ(applied.size(), 2U);
    EXPECT_FALSE(applied[1].down);
    EXPECT_FALSE(sink.IsActive());

    const auto finished = sink.GetProgress();
    EXPECT_FALSE(finished.active);
    EXPECT_EQ(
        finished.nextEventNanoseconds,
        std::numeric_limits<uint64_t>::max()
    );
    EXPECT_EQ(finished.appliedEvents, 2U);
    EXPECT_EQ(finished.totalEvents, 2U);
}

TEST(ReplayInputSinkTests, CpuSnapshotsDoNotApplyInput)
{
    const auto bundle = TestBundle();
    std::vector<ReplayEvent> applied;

    ReplayInputSink sink;
    sink.Start(bundle, [&applied](const ReplayEvent& event) {
        applied.push_back(event);
    });

    sink.SetCpuState(CpuAt(250));

    EXPECT_TRUE(applied.empty());
    EXPECT_EQ(sink.GetProgress().currentNanoseconds, 250U);
}

TEST(ReplayInputSinkTests, AcceptsBundlesLoadedFromJson)
{
    const auto serialized = boost::json::serialize(TestBundle().ToJson());
    const auto bundle = ReplayBundle::Parse(serialized);

    ReplayInputSink sink;
    ASSERT_NO_THROW(sink.Start(bundle, [](const ReplayEvent&) {}));

    const auto progress = sink.GetProgress();
    EXPECT_TRUE(progress.active);
    EXPECT_EQ(progress.currentNanoseconds, 100U);
    EXPECT_EQ(progress.stopNanoseconds, 300U);
}

TEST(ReplayInputSinkTests, IgnoresCpuSnapshotsWhenIdle)
{
    std::vector<ReplayEvent> applied;

    ReplayInputSink sink;
    sink.Start(TestBundle(), [&applied](const ReplayEvent& event) {
        applied.push_back(event);
    });
    sink.Stop();

    sink.SetCpuState(CpuAt(250));
    sink.OnTickBoundary(250);

    EXPECT_TRUE(applied.empty());
    EXPECT_FALSE(sink.IsActive());
}
