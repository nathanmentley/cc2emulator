#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <Compucolor/Debug/IDebugSink.h>

#include <Compucolor/App/DebugHub.h>

namespace
{
    using Compucolor::Debug::CpuState;
    using Compucolor::Debug::IDebugSink;
    using Compucolor::Debug::InstructionEvent;
    using Compucolor::Debug::PortEvent;
    using Compucolor::Debug::Smc5027State;
    using Compucolor::Debug::Tms5501State;

    class FakeSink: public IDebugSink {
        public:
            explicit FakeSink(const bool enabled = true): _enabled(enabled)
            {
            }

            [[nodiscard]] bool IsEnabled() const override
            {
                return _enabled;
            }

            void SetCpuState(const CpuState& state) override
            {
                ++cpuStateCalls;
                lastCpuState = state;
            }

            void SetCurrentPc(const uint16_t pc) override
            {
                ++currentPcCalls;
                lastPc = pc;
            }

            [[nodiscard]] uint16_t GetCurrentPc() const override
            {
                return lastPc;
            }

            void OnInstruction(const InstructionEvent& event) override
            {
                ++instructionCalls;
                lastInstruction = event;
            }

            void OnPort(const PortEvent& event) override
            {
                ++portCalls;
                lastPort = event;
            }

            void SetTms5501State(const Tms5501State& state) override
            {
                ++tms5501Calls;
                lastTms5501State = state;
            }

            void SetSmc5027State(const Smc5027State& state) override
            {
                ++smc5027Calls;
                lastSmc5027State = state;
            }

            bool _enabled;
            int cpuStateCalls = 0;
            int currentPcCalls = 0;
            int instructionCalls = 0;
            int portCalls = 0;
            int tms5501Calls = 0;
            int smc5027Calls = 0;
            CpuState lastCpuState{};
            InstructionEvent lastInstruction{};
            PortEvent lastPort{};
            Tms5501State lastTms5501State{};
            Smc5027State lastSmc5027State{};
            uint16_t lastPc = 0;
    };

    // Adds another sink from inside a callback to exercise the list-copy
    // behaviour: this must not deadlock and the new sink must miss that event.
    class AddingSink final: public FakeSink {
        public:
            AddingSink(
                Compucolor::Impl::App::DebugHub* hub,
                std::shared_ptr<IDebugSink> added
            ):
                _hub(hub),
                _added(std::move(added))
            {
            }

            void OnInstruction(const InstructionEvent& event) override
            {
                FakeSink::OnInstruction(event);
                _hub->AddSink(_added);
            }

        private:
            Compucolor::Impl::App::DebugHub* _hub;
            std::shared_ptr<IDebugSink> _added;
    };

    class RecordableFakeSink final:
        public FakeSink,
        public Compucolor::Impl::App::IRecordableDebugSink {
        public:
            void SetEnabled(const bool enabled) override
            {
                _enabled = enabled;
                enabledValues.push_back(enabled);
            }

            std::vector<bool> enabledValues;
    };
}

TEST(DebugHub, ForwardsEventsAndStateWhileEnabled)
{
    Compucolor::Impl::App::DebugHub hub;
    auto sink = std::make_shared<FakeSink>();
    hub.AddSink(sink);

    EXPECT_FALSE(hub.IsEnabled());
    hub.SetEnabled(true);
    EXPECT_TRUE(hub.IsEnabled());

    CpuState cpu;
    cpu.pc = 0x1234;
    cpu.a = 0x56;
    Tms5501State tms;
    tms.status = 0x81;
    Smc5027State smc;
    smc.cursorX = 12;

    InstructionEvent instruction;
    instruction.before.pc = 0x1234;
    instruction.bytes = {0x3E, 0x42, 0x00};
    instruction.cycles = 7;

    PortEvent port;
    port.pc = 0x1234;
    port.port = 0x10;
    port.value = 0xAB;
    port.isWrite = true;

    hub.SetCurrentPc(0x1234);
    hub.SetCpuState(cpu);
    hub.OnInstruction(instruction);
    hub.OnPort(port);
    hub.SetTms5501State(tms);
    hub.SetSmc5027State(smc);

    EXPECT_EQ(1, sink->currentPcCalls);
    EXPECT_EQ(0x1234, sink->lastPc);
    EXPECT_EQ(1, sink->cpuStateCalls);
    EXPECT_EQ(0x1234, sink->lastCpuState.pc);
    EXPECT_EQ(1, sink->instructionCalls);
    EXPECT_EQ(0x3E, sink->lastInstruction.bytes[0]);
    EXPECT_EQ(1, sink->portCalls);
    EXPECT_EQ(0x10, sink->lastPort.port);
    EXPECT_EQ(1, sink->tms5501Calls);
    EXPECT_EQ(0x81, sink->lastTms5501State.status);
    EXPECT_EQ(1, sink->smc5027Calls);
    EXPECT_EQ(12, sink->lastSmc5027State.cursorX);
}

TEST(DebugHub, IgnoresEventsButKeepsStateAndCurrentPcWhileDisabled)
{
    Compucolor::Impl::App::DebugHub hub;
    auto sink = std::make_shared<FakeSink>(false);
    hub.AddSink(sink);

    CpuState cpu;
    cpu.pc = 0x1234;
    Tms5501State tms;
    Smc5027State smc;

    hub.SetCurrentPc(0x1234);
    hub.SetCpuState(cpu);
    hub.OnInstruction(InstructionEvent{});
    hub.OnPort(PortEvent{});
    hub.SetTms5501State(tms);
    hub.SetSmc5027State(smc);

    EXPECT_EQ(0, sink->instructionCalls);
    EXPECT_EQ(0, sink->portCalls);
    EXPECT_EQ(1, sink->currentPcCalls);
    EXPECT_EQ(1, sink->cpuStateCalls);
    EXPECT_EQ(1, sink->tms5501Calls);
    EXPECT_EQ(1, sink->smc5027Calls);

    // The master gate is independent of the individual sink's own setting,
    // and the current PC is remembered even with no sinks attached.
    hub.SetEnabled(true);
    EXPECT_TRUE(hub.IsEnabled());

    Compucolor::Impl::App::DebugHub emptyHub;
    emptyHub.SetCurrentPc(0x4321);
    EXPECT_EQ(0x4321, emptyHub.GetCurrentPc());
}

TEST(DebugHub, FansOutToAllSinksAndIgnoresNullSinks)
{
    Compucolor::Impl::App::DebugHub hub;
    auto first = std::make_shared<FakeSink>();
    auto second = std::make_shared<FakeSink>();

    hub.AddSink(nullptr);
    hub.AddSink(first);
    hub.AddSink(second);
    hub.SetEnabled(true);

    hub.OnInstruction(InstructionEvent{});
    hub.OnPort(PortEvent{});

    EXPECT_EQ(1, first->instructionCalls);
    EXPECT_EQ(1, first->portCalls);
    EXPECT_EQ(1, second->instructionCalls);
    EXPECT_EQ(1, second->portCalls);
}

TEST(DebugHub, ListenersAreCalledOutsideTheSinkListLock)
{
    Compucolor::Impl::App::DebugHub hub;
    auto added = std::make_shared<FakeSink>();
    auto adder = std::make_shared<AddingSink>(&hub, added);

    hub.AddSink(adder);
    hub.SetEnabled(true);
    hub.OnInstruction(InstructionEvent{});

    EXPECT_EQ(1, adder->instructionCalls);
    EXPECT_EQ(0, added->instructionCalls);

    hub.OnInstruction(InstructionEvent{});
    EXPECT_EQ(1, added->instructionCalls);
}

TEST(DebugHub, PropagatesMasterEnableToRecordableSinks)
{
    Compucolor::Impl::App::DebugHub hub;
    auto sink = std::make_shared<RecordableFakeSink>();

    hub.AddSink(sink);
    ASSERT_EQ(1U, sink->enabledValues.size());
    EXPECT_FALSE(sink->enabledValues[0]);

    hub.SetEnabled(true);
    ASSERT_EQ(2U, sink->enabledValues.size());
    EXPECT_TRUE(sink->enabledValues[1]);

    hub.SetEnabled(false);
    ASSERT_EQ(3U, sink->enabledValues.size());
    EXPECT_FALSE(sink->enabledValues[2]);
}
