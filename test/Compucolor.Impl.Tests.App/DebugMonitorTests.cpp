#include <array>
#include <memory>

#include <gtest/gtest.h>

#include <Compucolor/Memory/IMemory.h>

#include "DebugMonitor.h"

namespace
{
    class FakeMemory: public Compucolor::Memory::IMemory {
        public:
            uint8_t GetByte(uint16_t address) override
            {
                return bytes[address];
            }

            void SetByte(uint16_t address, uint8_t data) override
            {
                bytes[address] = data;
            }

            void Start() override {}
            void Stop() override {}

            std::unique_ptr<boost::json::value> SaveState() override
            {
                return nullptr;
            }

            void LoadState(std::shared_ptr<boost::json::value>) override {}

        private:
            std::array<uint8_t, 65536> bytes{};
    };
}

TEST(DebugMonitor, BuffersEventsAndCpuStateWhileEnabled)
{
    auto monitor = Compucolor::Impl::App::Sfml::DebugMonitor(
        std::make_shared<FakeMemory>()
    );

    Compucolor::Debug::CpuState state;
    state.pc = 0x1234;
    state.a = 0x56;

    Compucolor::Debug::InstructionEvent instruction;
    instruction.before.pc = 0x1234;
    instruction.bytes = {0x3E, 0x42, 0x00};
    instruction.cycles = 7;

    Compucolor::Debug::PortEvent port;
    port.pc = 0x1234;
    port.port = 0x10;
    port.value = 0xAB;
    port.isWrite = true;

    monitor.SetEnabled(true);
    monitor.SetCpuState(state);
    monitor.SetCurrentPc(0x1234);
    monitor.OnInstruction(instruction);
    port.pc = monitor.GetCurrentPc();
    monitor.OnPort(port);

    EXPECT_EQ(0x1234, monitor.GetCpuState().pc);
    EXPECT_EQ(0x56, monitor.GetCpuState().a);

    const auto instructions = monitor.GetInstructions();
    ASSERT_EQ(1U, instructions.size());
    EXPECT_EQ(1U, instructions[0].sequence);
    EXPECT_EQ(0x1234, instructions[0].before.pc);
    EXPECT_EQ(0x3E, instructions[0].bytes[0]);
    EXPECT_EQ(7U, instructions[0].cycles);

    const auto ports = monitor.GetPorts();
    ASSERT_EQ(1U, ports.size());
    EXPECT_EQ(2U, ports[0].sequence);
    EXPECT_EQ(0x1234, ports[0].pc);
    EXPECT_TRUE(ports[0].isWrite);
    EXPECT_EQ(0x10, ports[0].port);
    EXPECT_EQ(0xAB, ports[0].value);
}

TEST(DebugMonitor, IgnoresEventsWhenDisabledAndClearRemovesBufferedEvents)
{
    auto monitor = Compucolor::Impl::App::Sfml::DebugMonitor(
        std::make_shared<FakeMemory>()
    );

    Compucolor::Debug::InstructionEvent instruction;
    monitor.OnInstruction(instruction);
    monitor.OnPort(Compucolor::Debug::PortEvent{});

    EXPECT_TRUE(monitor.GetInstructions().empty());
    EXPECT_TRUE(monitor.GetPorts().empty());

    monitor.SetEnabled(true);
    monitor.OnInstruction(instruction);
    monitor.OnPort(Compucolor::Debug::PortEvent{});

    ASSERT_EQ(1U, monitor.GetInstructions().size());
    ASSERT_EQ(1U, monitor.GetPorts().size());

    monitor.Clear();
    EXPECT_TRUE(monitor.GetInstructions().empty());
    EXPECT_TRUE(monitor.GetPorts().empty());
}

TEST(DebugMonitor, ReadsAndWritesMemory)
{
    auto memory = std::make_shared<FakeMemory>();
    auto monitor = Compucolor::Impl::App::Sfml::DebugMonitor(memory);

    monitor.WriteMemory(0x4000, 0xC3);
    EXPECT_EQ(0xC3, monitor.ReadMemory(0x4000));
    EXPECT_EQ(0xC3, memory->GetByte(0x4000));
}

TEST(DebugMonitor, FiltersInstructionsByPcAndMnemonic)
{
    auto monitor = Compucolor::Impl::App::Sfml::DebugMonitor(
        std::make_shared<FakeMemory>()
    );

    Compucolor::Impl::App::Sfml::DebugFilter filter;
    filter.enabled = true;
    filter.filterByPc = true;
    filter.pcFrom = 0x1000;
    filter.pcTo = 0x1002;
    filter.text = "mvi";
    monitor.SetInstructionFilter(filter);

    Compucolor::Debug::InstructionEvent matching;
    matching.before.pc = 0x1001;
    matching.bytes = {0x3E, 0x42, 0x00};

    Compucolor::Debug::InstructionEvent outsideRange = matching;
    outsideRange.before.pc = 0x2000;

    Compucolor::Debug::InstructionEvent wrongOpcode = matching;
    wrongOpcode.bytes = {0xC3, 0x00, 0x00};

    monitor.SetEnabled(true);
    monitor.OnInstruction(matching);
    monitor.OnInstruction(outsideRange);
    monitor.OnInstruction(wrongOpcode);

    const auto instructions = monitor.GetInstructions();
    ASSERT_EQ(1U, instructions.size());
    EXPECT_EQ(0x1001, instructions[0].before.pc);
    EXPECT_EQ(0x3E, instructions[0].bytes[0]);
}

TEST(DebugMonitor, FiltersPortsByRangeAndDirection)
{
    auto monitor = Compucolor::Impl::App::Sfml::DebugMonitor(
        std::make_shared<FakeMemory>()
    );

    Compucolor::Impl::App::Sfml::DebugFilter filter;
    filter.enabled = true;
    filter.filterByPort = true;
    filter.portFrom = 0x10;
    filter.portTo = 0x1F;
    filter.direction = Compucolor::Impl::App::Sfml::DebugPortDirection::Read;
    monitor.SetPortFilter(filter);

    Compucolor::Debug::PortEvent matching;
    matching.port = 0x12;
    matching.isWrite = false;

    Compucolor::Debug::PortEvent wrongDirection = matching;
    wrongDirection.isWrite = true;

    Compucolor::Debug::PortEvent outsideRange = matching;
    outsideRange.port = 0x20;

    monitor.SetEnabled(true);
    monitor.OnPort(matching);
    monitor.OnPort(wrongDirection);
    monitor.OnPort(outsideRange);

    const auto ports = monitor.GetPorts();
    ASSERT_EQ(1U, ports.size());
    EXPECT_EQ(0x12, ports[0].port);
    EXPECT_FALSE(ports[0].isWrite);
}

TEST(DebugMonitor, StopOnMatchFreezesAStreamUntilRearmed)
{
    auto monitor = Compucolor::Impl::App::Sfml::DebugMonitor(
        std::make_shared<FakeMemory>()
    );

    Compucolor::Impl::App::Sfml::DebugFilter filter;
    filter.enabled = true;
    filter.stopOnMatch = true;
    monitor.SetInstructionFilter(filter);

    Compucolor::Debug::InstructionEvent event;
    event.before.pc = 0x1234;
    event.bytes = {0x00, 0x00, 0x00};

    monitor.SetEnabled(true);
    monitor.OnInstruction(event);
    monitor.OnInstruction(event);
    monitor.OnInstruction(event);

    EXPECT_TRUE(monitor.IsInstructionTriggered());
    EXPECT_EQ(1U, monitor.GetInstructions().size());

    monitor.RearmTriggers();
    EXPECT_FALSE(monitor.IsInstructionTriggered());

    monitor.OnInstruction(event);
    EXPECT_EQ(2U, monitor.GetInstructions().size());
}

TEST(DebugMonitor, StoresPeripheralSnapshots)
{
    auto monitor = Compucolor::Impl::App::Sfml::DebugMonitor(
        std::make_shared<FakeMemory>()
    );

    Compucolor::Debug::Tms5501State tms;
    tms.status = 0x20;
    tms.timers = {0x01, 0x02, 0x03, 0x04, 0x05};
    tms.audioLevel = true;
    monitor.SetTms5501State(tms);

    Compucolor::Debug::Smc5027State smc;
    smc.cursorX = 12;
    smc.cursorY = 7;
    smc.firstDisplayRow = 3;
    monitor.SetSmc5027State(smc);

    EXPECT_EQ(0x20, monitor.GetTms5501State().status);
    EXPECT_EQ(0x05, monitor.GetTms5501State().timers[4]);
    EXPECT_TRUE(monitor.GetTms5501State().audioLevel);
    EXPECT_EQ(12, monitor.GetSmc5027State().cursorX);
    EXPECT_EQ(7, monitor.GetSmc5027State().cursorY);
    EXPECT_EQ(3, monitor.GetSmc5027State().firstDisplayRow);
}
