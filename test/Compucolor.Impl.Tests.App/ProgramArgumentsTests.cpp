#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "ProgramArguments.h"

using Compucolor::Impl::App::Sfml::ParseProgramArguments;
using Compucolor::Impl::App::Sfml::ProgramArguments;

namespace
{
    ProgramArguments Parse(const std::vector<const char*>& arguments)
    {
        std::vector<const char*> argv;
        argv.push_back("cc2emu");
        argv.insert(argv.end(), arguments.begin(), arguments.end());

        return ParseProgramArguments(
            static_cast<int>(argv.size()),
            argv.data()
        );
    }
}

TEST(ProgramArgumentsTests, DefaultsKeepAllOptionalPathsEmpty)
{
    const auto arguments = Parse({});

    EXPECT_TRUE(arguments.traceJsonlPath.empty());
    EXPECT_TRUE(arguments.loadStatePath.empty());
    EXPECT_TRUE(arguments.replayBundlePath.empty());
    EXPECT_TRUE(arguments.recordReplayPath.empty());
}

TEST(ProgramArgumentsTests, ParsesSeparateFlagValues)
{
    const auto arguments = Parse({
        "--trace-jsonl", "trace.jsonl",
        "--load-state", "state.json",
        "--record-replay", "session.cc2replay.json"
    });

    EXPECT_EQ(arguments.traceJsonlPath, "trace.jsonl");
    EXPECT_EQ(arguments.loadStatePath, "state.json");
    EXPECT_TRUE(arguments.replayBundlePath.empty());
    EXPECT_EQ(arguments.recordReplayPath, "session.cc2replay.json");
}

TEST(ProgramArgumentsTests, ParsesEqualsForm)
{
    const auto arguments = Parse({
        "--replay-bundle=session.cc2replay.json",
        "--trace-jsonl=trace.jsonl"
    });

    EXPECT_EQ(arguments.traceJsonlPath, "trace.jsonl");
    EXPECT_TRUE(arguments.loadStatePath.empty());
    EXPECT_EQ(arguments.replayBundlePath, "session.cc2replay.json");
    EXPECT_TRUE(arguments.recordReplayPath.empty());
}

TEST(ProgramArgumentsTests, RejectsMissingValues)
{
    EXPECT_THROW(Parse({"--trace-jsonl"}), std::runtime_error);
    EXPECT_THROW(Parse({"--replay-bundle"}), std::runtime_error);
}

TEST(ProgramArgumentsTests, RejectsEmptyValues)
{
    EXPECT_THROW(Parse({"--load-state="}), std::runtime_error);
}

TEST(ProgramArgumentsTests, RejectsUnknownOptions)
{
    EXPECT_THROW(Parse({"--step"}), std::runtime_error);
}

TEST(ProgramArgumentsTests, RejectsDuplicateOptions)
{
    EXPECT_THROW(
        Parse({"--load-state", "one.json", "--load-state=two.json"}),
        std::runtime_error
    );
}

TEST(ProgramArgumentsTests, RejectsMutuallyExclusiveStartupOptions)
{
    EXPECT_THROW(
        Parse({
            "--load-state", "state.json",
            "--replay-bundle", "session.cc2replay.json"
        }),
        std::runtime_error
    );

    EXPECT_THROW(
        Parse({
            "--record-replay", "new.cc2replay.json",
            "--replay-bundle", "old.cc2replay.json"
        }),
        std::runtime_error
    );
}

TEST(ProgramArgumentsTests, ParsesRepeatedTraceFilters)
{
    const auto arguments = Parse({
        "--trace-filter", "instr:text=MOV",
        "--trace-filter=port:port=0x80-0x8f,dir=write"
    });

    EXPECT_TRUE(arguments.traceFilters.Allows("instr", 0, 0, false, "mov b,c"));
    EXPECT_TRUE(arguments.traceFilters.Allows("port", 0, 0x80, true));
    EXPECT_FALSE(arguments.traceFilters.Allows("cpu", 0));
    EXPECT_FALSE(arguments.traceFilters.Allows("port", 0, 0x90, true));
}

TEST(ProgramArgumentsTests, RejectsInvalidTraceFilter)
{
    EXPECT_THROW(
        Parse({"--trace-filter", "bogus:pc=1"}),
        std::runtime_error
    );
}
