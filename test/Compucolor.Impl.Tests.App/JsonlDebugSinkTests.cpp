#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/json.hpp>
#include <gtest/gtest.h>

#include <Compucolor/Debug/IDebugSink.h>

#include <Compucolor/App/JsonlDebugSink.h>

namespace
{
    using Compucolor::Debug::CpuState;
    using Compucolor::Debug::InstructionEvent;
    using Compucolor::Debug::PortEvent;
    using Compucolor::Debug::Smc5027State;
    using Compucolor::Debug::Tms5501State;

    std::filesystem::path MakeTempPath(const std::string& extension)
    {
        static std::atomic<uint64_t> counter{0};

        const auto now = std::chrono::steady_clock::now()
            .time_since_epoch().count();
        const auto name = "cc2-jsonl-debug-" + std::to_string(now) + "-"
            + std::to_string(counter.fetch_add(1)) + extension;

        return std::filesystem::temp_directory_path() / name;
    }

    std::string AsString(const boost::json::value& value)
    {
        return std::string(value.as_string());
    }
}

TEST(JsonlDebugSink, WritesValidJsonLinesAndFlushesOnDestruction)
{
    const auto path = MakeTempPath(".jsonl");

    {
        Compucolor::Impl::App::JsonlDebugSink sink(path.string());
        EXPECT_TRUE(sink.IsEnabled());

        CpuState cpu;
        cpu.pc = 0x1234;
        cpu.sp = 0x0100;
        cpu.a = 0x56;
        cpu.cycles = 7;
        cpu.emulatedNanoseconds = 12345;

        sink.SetCurrentPc(0x1234);
        EXPECT_EQ(0x1234, sink.GetCurrentPc());
        sink.SetCpuState(cpu);

        InstructionEvent instruction;
        instruction.before.pc = 0x1234;
        instruction.bytes = {0x3E, 0x42, 0x00};
        instruction.cycles = 7;
        sink.OnInstruction(instruction);

        PortEvent port;
        port.pc = 0x1234;
        port.port = 0x10;
        port.value = 0xAB;
        port.isWrite = true;
        sink.OnPort(port);

        Tms5501State tms;
        tms.status = 0x81;
        tms.timers = {1, 2, 3, 4, 5};
        sink.SetTms5501State(tms);

        Smc5027State smc;
        smc.cursorX = 12;
        smc.cursorY = 4;
        smc.firstDisplayRow = 3;
        sink.SetSmc5027State(smc);
    }

    std::ifstream input(path);
    ASSERT_TRUE(input.is_open());

    std::vector<boost::json::value> entries;
    std::string contents;
    std::string line;

    while (std::getline(input, line))
    {
        contents += line + '\n';
        ASSERT_FALSE(line.empty());
        entries.push_back(boost::json::parse(line));
    }

    EXPECT_NE(std::string::npos, contents.find("\"kind\":\"instr\""));
    EXPECT_NE(std::string::npos, contents.find("\"kind\":\"port\""));
    EXPECT_NE(std::string::npos, contents.find("\"kind\":\"cpu\""));
    ASSERT_EQ(5U, entries.size());

    const char* expectedKinds[] = {
        "cpu", "instr", "port", "tms5501", "smc5027"
    };

    for (std::size_t index = 0; index < entries.size(); ++index)
    {
        ASSERT_TRUE(entries[index].is_object());
        const auto& object = entries[index].as_object();
        EXPECT_EQ(1, object.at("v").as_int64());
        EXPECT_EQ(static_cast<int64_t>(index + 1), object.at("seq").as_int64());

        if (index < 5)
        {
            EXPECT_EQ(expectedKinds[index], AsString(object.at("kind")));
        }
    }

    const auto& cpu = entries[0].as_object();
    EXPECT_EQ("0x1234", AsString(cpu.at("pc")));
    EXPECT_EQ("0x0100", AsString(cpu.at("sp")));
    EXPECT_EQ("0x56", AsString(cpu.at("a")));
    EXPECT_EQ(12345, cpu.at("emulatedNanoseconds").as_int64());

    const auto& instruction = entries[1].as_object();
    EXPECT_EQ("0x1234", AsString(instruction.at("pc")));
    EXPECT_EQ(instruction.end(), instruction.find("text"));

    const auto& bytes = instruction.at("bytes").as_array();
    ASSERT_EQ(3U, bytes.size());
    EXPECT_EQ("0x3E", AsString(bytes[0]));
    EXPECT_EQ("0x42", AsString(bytes[1]));

    const auto& port = entries[2].as_object();
    EXPECT_EQ("0x1234", AsString(port.at("pc")));
    EXPECT_EQ("0x10", AsString(port.at("port")));
    EXPECT_EQ("0xAB", AsString(port.at("value")));
    EXPECT_TRUE(port.at("isWrite").as_bool());

    const auto& tms = entries[3].as_object();
    EXPECT_EQ("0x81", AsString(tms.at("status")));
    ASSERT_EQ(5U, tms.at("timers").as_array().size());

    const auto& smc = entries[4].as_object();
    EXPECT_EQ("0x0C", AsString(smc.at("cursorX")));
    EXPECT_EQ("0x04", AsString(smc.at("cursorY")));

    (void)std::filesystem::remove(path);
}

TEST(JsonlDebugSink, DisabledSinkDoesNotWriteRecords)
{
    const auto path = MakeTempPath(".jsonl");

    {
        Compucolor::Impl::App::JsonlDebugSink sink(path.string());
        sink.SetEnabled(false);
        EXPECT_FALSE(sink.IsEnabled());

        sink.SetCpuState(CpuState{});
        sink.OnInstruction(InstructionEvent{});
        sink.OnPort(PortEvent{});
        sink.SetTms5501State(Tms5501State{});
        sink.SetSmc5027State(Smc5027State{});
    }

    std::ifstream input(path, std::ios::binary | std::ios::ate);
    ASSERT_TRUE(input.is_open());
    EXPECT_EQ(0, input.tellg());

    (void)std::filesystem::remove(path);
}

TEST(JsonlDebugSink, ThrowsWhenFileCannotBeOpened)
{
    const auto path = MakeTempPath(".jsonl");
    const auto missing = path.parent_path()
        / ("cc2-jsonl-missing-" + path.filename().string())
        / "debug.jsonl";

    EXPECT_THROW(
        {
            Compucolor::Impl::App::JsonlDebugSink sink(missing.string());
            (void)sink;
        },
        std::runtime_error
    );
}

TEST(JsonlDebugSink, FiltersRecordsAndKeepsSequenceDense)
{
    using Compucolor::Impl::App::TraceFilterConfig;

    const auto path = MakeTempPath(".jsonl");
    const auto filters = TraceFilterConfig::Parse({
        "instr:text=mov",
        "port:port=0x0010,dir=write"
    });

    {
        Compucolor::Impl::App::JsonlDebugSink sink(path.string(), filters);

        CpuState cpu;
        cpu.pc = 0x1234;
        sink.SetCpuState(cpu);

        InstructionEvent mov;
        mov.before.pc = 0x0100;
        mov.bytes = {0x41, 0x00, 0x00};
        sink.OnInstruction(mov);

        InstructionEvent nop;
        nop.before.pc = 0x0101;
        nop.bytes = {0x00, 0x00, 0x00};
        sink.OnInstruction(nop);

        PortEvent write;
        write.pc = 0x0200;
        write.port = 0x10;
        write.value = 0xAB;
        write.isWrite = true;
        sink.OnPort(write);

        PortEvent read;
        read.pc = 0x0200;
        read.port = 0x10;
        read.value = 0xCD;
        read.isWrite = false;
        sink.OnPort(read);

        sink.SetTms5501State(Tms5501State{});
        sink.SetSmc5027State(Smc5027State{});
    }

    std::ifstream input(path);
    ASSERT_TRUE(input.is_open());

    std::vector<boost::json::value> entries;
    std::string line;

    while (std::getline(input, line))
    {
        ASSERT_FALSE(line.empty());
        entries.push_back(boost::json::parse(line));
    }

    ASSERT_EQ(2U, entries.size());
    EXPECT_EQ("instr", AsString(entries[0].as_object().at("kind")));
    EXPECT_EQ("0x0100", AsString(entries[0].as_object().at("pc")));
    EXPECT_EQ(1, entries[0].as_object().at("seq").as_int64());

    EXPECT_EQ("port", AsString(entries[1].as_object().at("kind")));
    EXPECT_EQ("0x10", AsString(entries[1].as_object().at("port")));
    EXPECT_TRUE(entries[1].as_object().at("isWrite").as_bool());
    EXPECT_EQ(2, entries[1].as_object().at("seq").as_int64());

    (void)std::filesystem::remove(path);
}

TEST(JsonlDebugSink, BareKindFilterOmitsOtherKinds)
{
    using Compucolor::Impl::App::TraceFilterConfig;

    const auto path = MakeTempPath(".jsonl");

    {
        Compucolor::Impl::App::JsonlDebugSink sink(
            path.string(),
            TraceFilterConfig::Parse({"smc5027"})
        );

        sink.SetCpuState(CpuState{});
        sink.OnInstruction(InstructionEvent{});
        sink.OnPort(PortEvent{});
        sink.SetTms5501State(Tms5501State{});

        Smc5027State smc;
        smc.cursorX = 7;
        sink.SetSmc5027State(smc);
    }

    std::ifstream input(path);
    ASSERT_TRUE(input.is_open());

    std::vector<boost::json::value> entries;
    std::string line;

    while (std::getline(input, line))
    {
        entries.push_back(boost::json::parse(line));
    }

    ASSERT_EQ(1U, entries.size());
    EXPECT_EQ("smc5027", AsString(entries[0].as_object().at("kind")));
    EXPECT_EQ(1, entries[0].as_object().at("seq").as_int64());

    (void)std::filesystem::remove(path);
}

TEST(JsonlDebugSink, FlushWritesBufferedRecordsBeforeClose)
{
    const auto path = MakeTempPath(".jsonl");

    Compucolor::Impl::App::JsonlDebugSink sink(path.string());

    CpuState cpu;
    cpu.pc = 0x1234;
    sink.SetCpuState(cpu);

    sink.Flush();

    std::ifstream input(path);
    ASSERT_TRUE(input.is_open());

    std::vector<boost::json::value> entries;
    std::string line;

    while (std::getline(input, line))
    {
        entries.push_back(boost::json::parse(line));
    }

    ASSERT_EQ(1U, entries.size());
    EXPECT_EQ("cpu", AsString(entries[0].as_object().at("kind")));

    (void)std::filesystem::remove(path);
}
