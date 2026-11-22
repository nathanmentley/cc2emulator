#include <array>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <Compucolor/App/Replay/InputTimeline.h>
#include <Compucolor/App/Replay/ReplayBundle.h>
#include <Compucolor/App/Replay/ReplayKeyNames.h>
#include <Compucolor/App/Replay/StateHash.h>

using Compucolor::Keyboard::CompucolorIIKey;
using Compucolor::Impl::App::Replay::InputTimeline;
using Compucolor::Impl::App::Replay::ReplayBundle;
using Compucolor::Impl::App::Replay::ReplayEvent;
using Compucolor::Impl::App::Replay::ReplayKeyNames;

namespace
{
    std::vector<CompucolorIIKey> AllKeys()
    {
        return {
#define CC2_TEST_KEY(name) CompucolorIIKey::name,
            CC2_REPLAY_KEYS(CC2_TEST_KEY)
#undef CC2_TEST_KEY
        };
    }

    std::string ValidBundleJson()
    {
        return R"({
            "version": 1,
            "createdBy": "test",
            "state": {
                "emulatedNanoseconds": 1000,
                "memory": {"bytes": []}
            },
            "input": [
                {"t": 1000, "key": "A", "down": true},
                {"t": 1500, "key": "Shift", "down": true}
            ],
            "stop": {"emulatedNanoseconds": 5000}
        })";
    }
}

TEST(ReplayKeyNamesTests, RoundTripsEveryKey)
{
    const auto keys = AllKeys();

    ASSERT_EQ(keys.size(), ReplayKeyNames::Count());

    for (const CompucolorIIKey key : keys)
    {
        const std::string name = ReplayKeyNames::ToString(key);
        const auto parsed = ReplayKeyNames::FromString(name);

        ASSERT_TRUE(parsed.has_value()) << name;
        EXPECT_EQ(parsed.value(), key) << name;
    }
}

TEST(ReplayKeyNamesTests, RejectsUnknownNames)
{
    EXPECT_FALSE(ReplayKeyNames::FromString("NotAKey").has_value());
}

TEST(ReplayBundleTests, ParsesValidBundle)
{
    const ReplayBundle bundle = ReplayBundle::Parse(ValidBundleJson());

    EXPECT_EQ(bundle.CreatedBy(), "test");
    EXPECT_EQ(bundle.StopNanoseconds(), 5000U);
    ASSERT_EQ(bundle.Input().size(), 2U);
    EXPECT_EQ(bundle.Input()[0].emulatedNanoseconds, 1000U);
    EXPECT_EQ(bundle.Input()[0].key, CompucolorIIKey::A);
    EXPECT_TRUE(bundle.Input()[0].down);
    EXPECT_EQ(bundle.Input()[1].key, CompucolorIIKey::Shift);
}

TEST(ReplayBundleTests, SerializesToParseableJson)
{
    const ReplayBundle original = ReplayBundle::Parse(ValidBundleJson());
    const ReplayBundle reparsed = ReplayBundle::Parse(
        boost::json::serialize(original.ToJson())
    );

    ASSERT_EQ(reparsed.Input().size(), original.Input().size());

    for (std::size_t i = 0; i < original.Input().size(); i++)
    {
        EXPECT_EQ(reparsed.Input()[i].emulatedNanoseconds,
                  original.Input()[i].emulatedNanoseconds);
        EXPECT_EQ(reparsed.Input()[i].key, original.Input()[i].key);
        EXPECT_EQ(reparsed.Input()[i].down, original.Input()[i].down);
    }
}

TEST(ReplayBundleTests, RejectsInvalidBundles)
{
    struct Case {
        std::string name;
        std::string json;
    };

    const std::array<Case, 5> cases{{
        {
            "version",
            R"({"version":2,"state":{"emulatedNanoseconds":0},"input":[],"stop":{"emulatedNanoseconds":10}})"
        },
        {
            "order",
            R"({"version":1,"state":{"emulatedNanoseconds":0},"input":[{"t":20,"key":"A","down":true},{"t":10,"key":"B","down":true}],"stop":{"emulatedNanoseconds":100}})"
        },
        {
            "key",
            R"({"version":1,"state":{"emulatedNanoseconds":0},"input":[{"t":10,"key":"Nope","down":true}],"stop":{"emulatedNanoseconds":100}})"
        },
        {
            "before state",
            R"({"version":1,"state":{"emulatedNanoseconds":50},"input":[{"t":10,"key":"A","down":true}],"stop":{"emulatedNanoseconds":100}})"
        },
        {
            "after stop",
            R"({"version":1,"state":{"emulatedNanoseconds":0},"input":[{"t":200,"key":"A","down":true}],"stop":{"emulatedNanoseconds":100}})"
        }
    }};

    for (const auto& testCase : cases)
    {
        EXPECT_THROW(
            ReplayBundle::Parse(testCase.json),
            std::runtime_error
        ) << testCase.name;
    }
}

TEST(StateHashTests, MatchesKnownSha256Vectors)
{
    EXPECT_EQ(
        Compucolor::Impl::App::Replay::StateHash::Sha256("abc"),
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
    );
    EXPECT_EQ(
        Compucolor::Impl::App::Replay::StateHash::Sha256(""),
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
    );
}

TEST(StateHashTests, CanonicalizesObjectKeyOrder)
{
    const auto first = boost::json::parse(R"({"a":1,"b":{"x":true,"y":2}})");
    const auto second = boost::json::parse(R"({"b":{"y":2,"x":true},"a":1})");

    EXPECT_EQ(
        Compucolor::Impl::App::Replay::StateHash::Sha256(
            boost::json::serialize(
                Compucolor::Impl::App::Replay::StateHash::Canonicalize(first)
            )
        ),
        Compucolor::Impl::App::Replay::StateHash::Sha256(
            boost::json::serialize(
                Compucolor::Impl::App::Replay::StateHash::Canonicalize(second)
            )
        )
    );
}

TEST(InputTimelineTests, AppliesDueEventsOnceInStableOrder)
{
    const std::vector<ReplayEvent> events{
        {10, CompucolorIIKey::A, true},
        {10, CompucolorIIKey::B, true},
        {20, CompucolorIIKey::A, false}
    };

    InputTimeline timeline(events);
    std::vector<CompucolorIIKey> applied;

    const auto apply = [&applied](const ReplayEvent& event) {
        applied.push_back(event.key);
    };

    timeline.ApplyDue(9, apply);
    EXPECT_TRUE(applied.empty());
    EXPECT_EQ(timeline.NextEventTime(), 10U);

    timeline.ApplyDue(10, apply);
    timeline.ApplyDue(15, apply);
    ASSERT_EQ(applied.size(), 2U);
    EXPECT_EQ(applied[0], CompucolorIIKey::A);
    EXPECT_EQ(applied[1], CompucolorIIKey::B);

    timeline.ApplyDue(100, apply);
    ASSERT_EQ(applied.size(), 3U);
    EXPECT_EQ(applied[2], CompucolorIIKey::A);
    EXPECT_FALSE(timeline.HasPending());
}
