#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <boost/json.hpp>

#include <Compucolor/App/DebugDisassembler.h>
#include <Compucolor/App/DebugHub.h>
#include <Compucolor/App/Replay/HeadlessEmulatorFactory.h>
#include <Compucolor/App/JsonlDebugSink.h>
#include <Compucolor/App/Replay/ReplayBundle.h>
#include <Compucolor/App/Replay/ReplayInputSink.h>
#include <Compucolor/App/Replay/ReplayRunner.h>
#include <Compucolor/App/Replay/StateHash.h>
#include <Compucolor/App/TraceFilterConfig.h>

using Compucolor::Keyboard::CompucolorIIKey;
using Compucolor::Impl::App::Replay::HeadlessMachine;
using Compucolor::Impl::App::Replay::ReplayBundle;
using Compucolor::Impl::App::Replay::ReplayEvent;
using Compucolor::Impl::App::Replay::ReplayInputSink;
using Compucolor::Impl::App::Replay::ReplayRunner;
using Compucolor::Impl::App::Replay::StateHash;
using Compucolor::Impl::App::TraceFilterConfig;

namespace
{
    constexpr char GoldenStateHash[] =
        "ea9677ef6c0a044abdf846275cf992384c68744124fa3d4ea4fe59d475019553";

    std::string ReadFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::in | std::ios::binary);

        return std::string{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };
    }

    std::filesystem::path TracePath(const std::string& suffix)
    {
        static std::atomic<uint64_t> counter{0};
        const auto stamp = std::chrono::steady_clock::now()
            .time_since_epoch()
            .count();

        return std::filesystem::temp_directory_path() /
            ("cc2-replay-" + std::to_string(stamp) + "-" +
             std::to_string(counter++) + "-" + suffix + ".jsonl");
    }

    boost::json::value CreateSeedState()
    {
        const HeadlessMachine machine = Compucolor::Impl::App::Replay::CreateHeadlessMachine();

        machine.emulator->Start();
        machine.emulator->SetSpeedRegulated(false);
        machine.scheduler->Advance(3'000'000);

        auto state = machine.emulator->SaveState();

        machine.emulator->Stop();

        if (!state)
        {
            throw std::runtime_error("Seed machine returned no state");
        }

        return *state;
    }

    ReplayBundle CreateBundle()
    {
        boost::json::value state = CreateSeedState();
        const uint64_t baseline = state.as_object()
            .at("emulatedNanoseconds")
            .as_uint64();

        const std::vector<ReplayEvent> input{
            {baseline, CompucolorIIKey::A, true},
            {baseline + 500, CompucolorIIKey::Shift, true},
            {baseline + 700, CompucolorIIKey::A, false}
        };

        return ReplayBundle(
            state,
            input,
            baseline + 1'000'000,
            "ReplayRunnerTests"
        );
    }

    struct RunResult {
        std::string trace;
        std::string hash;
        uint64_t finalNanoseconds = 0;
    };

    RunResult RunBundle(
        const ReplayBundle& bundle,
        const std::filesystem::path& tracePath,
        const TraceFilterConfig& filters = {}
    )
    {
        RunResult result;

        {
            const HeadlessMachine machine =
                Compucolor::Impl::App::Replay::CreateHeadlessMachine();
            auto hub = std::make_shared<Compucolor::Impl::App::DebugHub>();
            auto sink = std::make_shared<Compucolor::Impl::App::JsonlDebugSink>(
                tracePath.string(),
                filters
            );

            hub->AddSink(sink);

            ReplayRunner runner(machine.emulator, machine.scheduler, hub);
            runner.Run(bundle);
            sink->Flush();

            result.hash = StateHash::OfState(runner.FinalState());
            result.finalNanoseconds = runner.FinalEmulatedNanoseconds();
        }

        result.trace = ReadFile(tracePath);
        std::filesystem::remove(tracePath);

        return result;
    }

    RunResult RunBundleThroughInputSink(
        const ReplayBundle& bundle,
        const std::filesystem::path& tracePath,
        const TraceFilterConfig& filters = {}
    )
    {
        RunResult result;

        const HeadlessMachine machine = Compucolor::Impl::App::Replay::CreateHeadlessMachine();
        auto hub = std::make_shared<Compucolor::Impl::App::DebugHub>();
        auto sink = std::make_shared<Compucolor::Impl::App::JsonlDebugSink>(
            tracePath.string(),
            filters
        );
        auto replaySink = std::make_shared<ReplayInputSink>();

        hub->AddSink(replaySink);
        hub->AddSink(sink);
        machine.emulator->SetDebugSink(hub);

        machine.emulator->Start();
        machine.emulator->LoadState(
            std::make_shared<boost::json::value>(bundle.State())
        );
        machine.emulator->SetSpeedRegulated(false);

        replaySink->Start(
            bundle,
            [&machine](const ReplayEvent& event) {
                if (event.down)
                {
                    machine.emulator->OnKeyDown(event.key);
                }
                else
                {
                    machine.emulator->OnKeyUp(event.key);
                }
            }
        );

        hub->SetEnabled(true);

        uint64_t current = machine.emulator->GetEmulatedNanoseconds();

        while (current < bundle.StopNanoseconds())
        {
            machine.scheduler->Advance(
                Compucolor::Impl::App::Replay::ManualScheduler::Interval()
            );
            current = machine.emulator->GetEmulatedNanoseconds();
        }

        replaySink->Stop();
        sink->Flush();

        auto state = machine.emulator->SaveState();

        if (!state)
        {
            throw std::runtime_error("Input sink machine returned no state");
        }

        result.hash = StateHash::OfState(*state);
        result.finalNanoseconds = current;
        result.trace = ReadFile(tracePath);
        std::filesystem::remove(tracePath);

        machine.emulator->Stop();

        return result;
    }

    std::vector<boost::json::value> ParseTrace(const std::string& trace)
    {
        std::vector<boost::json::value> entries;
        std::istringstream input(trace);
        std::string line;

        while (std::getline(input, line))
        {
            if (!line.empty())
            {
                entries.push_back(boost::json::parse(line));
            }
        }

        return entries;
    }

    std::string LowerCopy(std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](const unsigned char character) {
                return static_cast<char>(std::tolower(character));
            }
        );

        return value;
    }

    uint16_t HexValue(const boost::json::value& value)
    {
        return static_cast<uint16_t>(
            std::stoul(std::string(value.as_string()), nullptr, 16)
        );
    }

    std::string Signature(const boost::json::value& entry)
    {
        boost::json::object copy;

        for (const auto& pair : entry.as_object())
        {
            if (pair.key() != "v" && pair.key() != "seq")
            {
                copy.emplace(pair.key(), pair.value());
            }
        }

        return boost::json::serialize(copy);
    }

    std::string InstructionText(const boost::json::value& entry)
    {
        const auto& object = entry.as_object();
        std::array<uint8_t, 3> bytes{};
        const auto& values = object.at("bytes").as_array();

        for (std::size_t index = 0; index < bytes.size(); ++index)
        {
            bytes[index] = static_cast<uint8_t>(HexValue(values[index]));
        }

        return Compucolor::Impl::App::Disassemble8080(
            bytes.data(),
            HexValue(object.at("pc"))
        ).text;
    }

    bool MatchesGoldenFilter(const boost::json::value& entry)
    {
        const auto& object = entry.as_object();
        const std::string kind(object.at("kind").as_string());

        if (kind == "instr")
        {
            return LowerCopy(InstructionText(entry)).find("mov") !=
                std::string::npos;
        }

        if (kind == "port")
        {
            const uint16_t port = HexValue(object.at("port"));
            return port >= 0x80 && port <= 0x8F &&
                object.at("isWrite").as_bool();
        }

        return false;
    }

    std::vector<std::string> SignaturesOf(
        const std::vector<boost::json::value>& entries
    )
    {
        std::vector<std::string> signatures;

        for (const auto& entry : entries)
        {
            signatures.push_back(Signature(entry));
        }

        return signatures;
    }
}

TEST(ReplayRunnerTests, RepeatsTraceAndFinalStateDeterministically)
{
    const ReplayBundle bundle = CreateBundle();
    const auto firstPath = TracePath("first");
    const auto secondPath = TracePath("second");

    const RunResult first = RunBundle(bundle, firstPath);
    const RunResult second = RunBundle(bundle, secondPath);

    ASSERT_FALSE(first.trace.empty());
    EXPECT_EQ(first.trace, second.trace);
    EXPECT_EQ(first.hash, second.hash);
    EXPECT_GT(first.finalNanoseconds, bundle.StopNanoseconds());

    EXPECT_EQ(first.hash, GoldenStateHash);
}

TEST(ReplayRunnerTests, FilteredTraceMatchesExpectedRecordsWithDenseSequence)
{
    const ReplayBundle bundle = CreateBundle();
    const auto filters = TraceFilterConfig::Parse({
        "instr:text=MOV",
        "port:port=0x80-0x8f,dir=write"
    });

    const RunResult unfiltered = RunBundle(bundle, TracePath("unfiltered"));
    const RunResult filtered = RunBundle(bundle, TracePath("filtered"), filters);
    const RunResult repeated = RunBundle(bundle, TracePath("filtered-again"), filters);

    const auto unfilteredEntries = ParseTrace(unfiltered.trace);
    const auto filteredEntries = ParseTrace(filtered.trace);

    std::vector<std::string> expected;

    for (const auto& entry : unfilteredEntries)
    {
        if (MatchesGoldenFilter(entry))
        {
            expected.push_back(Signature(entry));
        }
    }

    ASSERT_FALSE(expected.empty());
    ASSERT_EQ(expected.size(), filteredEntries.size());
    EXPECT_EQ(filtered.trace, repeated.trace);
    EXPECT_EQ(filtered.hash, GoldenStateHash);

    for (std::size_t index = 0; index < filteredEntries.size(); ++index)
    {
        const auto& entry = filteredEntries[index].as_object();
        EXPECT_EQ(static_cast<int64_t>(index + 1), entry.at("seq").as_int64());
    }

    EXPECT_EQ(expected, SignaturesOf(filteredEntries));
}

TEST(ReplayRunnerTests, InputSinkPathIgnoresJsonlFilters)
{
    const ReplayBundle bundle = CreateBundle();
    const auto filters = TraceFilterConfig::Parse({
        "instr:text=MOV",
        "port:port=0x80-0x8f,dir=write"
    });

    const RunResult unfiltered = RunBundleThroughInputSink(
        bundle,
        TracePath("input-sink-unfiltered")
    );
    const RunResult filtered = RunBundleThroughInputSink(
        bundle,
        TracePath("input-sink-filtered"),
        filters
    );

    EXPECT_EQ(unfiltered.hash, filtered.hash);
    EXPECT_EQ(unfiltered.finalNanoseconds, filtered.finalNanoseconds);
}
