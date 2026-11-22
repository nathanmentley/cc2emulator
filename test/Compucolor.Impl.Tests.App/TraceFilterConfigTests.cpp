#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <Compucolor/App/TraceFilterConfig.h>

using Compucolor::Impl::App::TraceFilterConfig;

namespace
{
    void ExpectInvalid(const std::string& spec, const std::string& messagePart)
    {
        try
        {
            (void)TraceFilterConfig::Parse({spec});
            FAIL() << "Expected invalid filter: " << spec;
        }
        catch (const std::runtime_error& error)
        {
            EXPECT_NE(
                std::string(error.what()).find(spec),
                std::string::npos
            );
            EXPECT_NE(
                std::string(error.what()).find(messagePart),
                std::string::npos
            );
        }
    }
}

TEST(TraceFilterConfigTests, EmptyConfigAllowsEverything)
{
    const auto config = TraceFilterConfig::Parse({});

    EXPECT_TRUE(config.Empty());
    EXPECT_TRUE(config.Allows("cpu", 0x1234));
    EXPECT_TRUE(config.Allows("instr", 0x1234));
    EXPECT_TRUE(config.Allows("port", 0x1234, 0x80, true));
    EXPECT_TRUE(config.Allows("tms5501"));
    EXPECT_TRUE(config.Allows("smc5027"));
}

TEST(TraceFilterConfigTests, UnmentionedKindsAreOmitted)
{
    const auto config = TraceFilterConfig::Parse({"cpu:pc=0x0100"});

    EXPECT_FALSE(config.Empty());
    EXPECT_TRUE(config.Allows("cpu", 0x0100));
    EXPECT_FALSE(config.Allows("cpu", 0x0200));
    EXPECT_FALSE(config.Allows("instr", 0x0100));
    EXPECT_FALSE(config.Allows("port", 0x0100, 0x80, true));
    EXPECT_FALSE(config.Allows("tms5501"));
    EXPECT_FALSE(config.Allows("smc5027"));
}

TEST(TraceFilterConfigTests, ParsesSingleAddressesAndInclusiveRanges)
{
    const auto single = TraceFilterConfig::Parse({"instr:pc=0100"});
    EXPECT_TRUE(single.Allows("instr", 0x0100));
    EXPECT_FALSE(single.Allows("instr", 0x0101));

    const auto range = TraceFilterConfig::Parse({"instr:pc=0x0100-0x01ff"});
    EXPECT_TRUE(range.Allows("instr", 0x0100));
    EXPECT_TRUE(range.Allows("instr", 0x01ff));
    EXPECT_FALSE(range.Allows("instr", 0x0200));
}

TEST(TraceFilterConfigTests, MatchesTextCaseInsensitively)
{
    const auto config = TraceFilterConfig::Parse({"instr:text=mov"});

    EXPECT_TRUE(config.WantsText("instr"));
    EXPECT_FALSE(config.WantsText("cpu"));
    EXPECT_TRUE(config.Allows("instr", 0, 0, false, "MOV B,C"));
    EXPECT_TRUE(config.Allows("instr", 0, 0, false, "mov a,l"));
    EXPECT_FALSE(config.Allows("instr", 0, 0, false, "NOP"));
    EXPECT_FALSE(config.Allows("instr", 0, 0, false, ""));
}

TEST(TraceFilterConfigTests, MatchesPortRangeAndDirection)
{
    const auto writes = TraceFilterConfig::Parse(
        {"port:port=0x80-0x8f,dir=write"}
    );

    EXPECT_TRUE(writes.Allows("port", 0x1234, 0x80, true));
    EXPECT_TRUE(writes.Allows("port", 0x1234, 0x8F, true));
    EXPECT_FALSE(writes.Allows("port", 0x1234, 0x90, true));
    EXPECT_FALSE(writes.Allows("port", 0x1234, 0x80, false));

    const auto reads = TraceFilterConfig::Parse({"port:dir=read"});
    EXPECT_TRUE(reads.Allows("port", 0, 0x10, false));
    EXPECT_FALSE(reads.Allows("port", 0, 0x10, true));

    const auto any = TraceFilterConfig::Parse({"port:dir=ANY"});
    EXPECT_TRUE(any.Allows("port", 0, 0x10, false));
    EXPECT_TRUE(any.Allows("port", 0, 0x10, true));
}

TEST(TraceFilterConfigTests, MultipleFiltersForSameKindAreOred)
{
    const auto config = TraceFilterConfig::Parse({
        "instr:text=MOV",
        "instr:pc=0x0200"
    });

    EXPECT_TRUE(config.Allows("instr", 0x0100, 0, false, "MOV B,C"));
    EXPECT_TRUE(config.Allows("instr", 0x0200, 0, false, "NOP"));
    EXPECT_FALSE(config.Allows("instr", 0x0100, 0, false, "NOP"));
}

TEST(TraceFilterConfigTests, BarePeripheralKindsAreSupported)
{
    const auto config = TraceFilterConfig::Parse({
        "cpu",
        "smc5027"
    });

    EXPECT_TRUE(config.Allows("cpu", 0xFFFF));
    EXPECT_TRUE(config.Allows("smc5027"));
    EXPECT_FALSE(config.Allows("tms5501"));
}

TEST(TraceFilterConfigTests, RejectsInvalidSpecsAtParseTime)
{
    ExpectInvalid("", "filter is empty");
    ExpectInvalid("bogus", "unknown kind");
    ExpectInvalid("instr:", "missing keys");
    ExpectInvalid("instr:wat=1", "not valid");
    ExpectInvalid("cpu:text=MOV", "not valid");
    ExpectInvalid("port:dir=sideways", "dir must be");
    ExpectInvalid("port:port=0x100", "out of range");
    ExpectInvalid("instr:pc=0x0200-0x0100", "starts after");
    ExpectInvalid("instr:pc=xyz", "hex address");
    ExpectInvalid("instr:pc=0x0100,", "empty key segment");
    ExpectInvalid("instr:pc=0x0100,pc=0x0200", "duplicate key");
    ExpectInvalid("port:=5", "expected key=value");
}

TEST(TraceFilterConfigTests, RejectsMissingValues)
{
    EXPECT_THROW(
        (void)TraceFilterConfig::Parse({"instr:text="}),
        std::runtime_error
    );
}
