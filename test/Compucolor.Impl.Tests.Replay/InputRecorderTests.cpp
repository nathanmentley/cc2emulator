#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>

#include <boost/json.hpp>

#include <Compucolor/App/Replay/InputRecorder.h>
#include <Compucolor/App/Replay/ReplayBundle.h>

using Compucolor::Keyboard::CompucolorIIKey;
using Compucolor::Impl::App::Replay::InputRecorder;
using Compucolor::Impl::App::Replay::ReplayBundle;

namespace
{
    boost::json::value TestState(const uint64_t nanoseconds)
    {
        boost::json::object state;
        state["emulatedNanoseconds"] = nanoseconds;
        return boost::json::value(std::move(state));
    }
}

TEST(InputRecorderTests, IgnoresEventsUntilStarted)
{
    InputRecorder recorder;

    recorder.Record(true, CompucolorIIKey::A, 100);

    EXPECT_FALSE(recorder.IsRecording());
    EXPECT_THROW((void)recorder.BuildBundle(200), std::runtime_error);
}

TEST(InputRecorderTests, RecordsEventsAndBuildsReplayableBundle)
{
    InputRecorder recorder;
    recorder.Start(TestState(100), 100);

    EXPECT_TRUE(recorder.IsRecording());

    recorder.Record(true, CompucolorIIKey::A, 150);
    recorder.Record(false, CompucolorIIKey::A, 200);

    const auto bundle = recorder.BuildBundle(300, "test");
    const auto parsed = ReplayBundle::Parse(
        boost::json::serialize(bundle.ToJson())
    );

    ASSERT_EQ(parsed.Input().size(), 2U);
    EXPECT_EQ(parsed.Input()[0].emulatedNanoseconds, 150U);
    EXPECT_EQ(parsed.Input()[0].key, CompucolorIIKey::A);
    EXPECT_TRUE(parsed.Input()[0].down);
    EXPECT_EQ(parsed.Input()[1].emulatedNanoseconds, 200U);
    EXPECT_FALSE(parsed.Input()[1].down);
    EXPECT_EQ(parsed.StopNanoseconds(), 300U);
    EXPECT_EQ(parsed.CreatedBy(), "test");
}

TEST(InputRecorderTests, ClampsOutOfOrderTimestamps)
{
    InputRecorder recorder;
    recorder.Start(TestState(100), 100);

    recorder.Record(true, CompucolorIIKey::Shift, 200);
    recorder.Record(true, CompucolorIIKey::A, 150);
    recorder.Record(false, CompucolorIIKey::A, 120);

    const auto bundle = recorder.BuildBundle(300);

    ASSERT_EQ(bundle.Input().size(), 3U);
    EXPECT_EQ(bundle.Input()[0].emulatedNanoseconds, 200U);
    EXPECT_EQ(bundle.Input()[1].emulatedNanoseconds, 200U);
    EXPECT_EQ(bundle.Input()[2].emulatedNanoseconds, 200U);
}

TEST(InputRecorderTests, ZeroLengthBundleGetsValidStopTime)
{
    InputRecorder recorder;
    recorder.Start(TestState(100), 100);

    const auto bundle = recorder.BuildBundle(100);

    EXPECT_EQ(bundle.StopNanoseconds(), 101U);
    EXPECT_NO_THROW(
        ReplayBundle::Parse(boost::json::serialize(bundle.ToJson()))
    );
}

TEST(InputRecorderTests, StopPreventsFurtherRecording)
{
    InputRecorder recorder;
    recorder.Start(TestState(100), 100);
    recorder.Record(true, CompucolorIIKey::A, 150);
    recorder.Stop();

    EXPECT_FALSE(recorder.IsRecording());
    EXPECT_THROW((void)recorder.BuildBundle(200), std::runtime_error);

    recorder.Record(false, CompucolorIIKey::A, 250);
    EXPECT_FALSE(recorder.IsRecording());
}
