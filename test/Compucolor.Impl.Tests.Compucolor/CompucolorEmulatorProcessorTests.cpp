#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iterator>
#include <memory>
#include <string>
#include <thread>

#include <boost/json.hpp>

#include <Compucolor/CompucolorEmulatorProcessor.h>
#include <Compucolor/ICompucolorEmulator.h>

using Compucolor::CompucolorEmulatorProcessor;
using Compucolor::ICompucolorEmulator;

using ::testing::NiceMock;
using ::testing::SaveArg;
using ::testing::_;

namespace
{
    class FakeEmulator: public ICompucolorEmulator {
        public:
            MOCK_METHOD(void, Start, (), (override));
            MOCK_METHOD(void, Stop, (), (override));
            MOCK_METHOD(std::unique_ptr<boost::json::value>, SaveState, (), (override));
            MOCK_METHOD(void, LoadState, (std::shared_ptr<boost::json::value> state), (override));
            MOCK_METHOD(void, OnKeyUp, (Compucolor::Keyboard::CompucolorIIKey key), (override));
            MOCK_METHOD(void, OnKeyDown, (Compucolor::Keyboard::CompucolorIIKey key), (override));
            MOCK_METHOD(void, Reset, (bool hard), (override));
            MOCK_METHOD(void, Pause, (), (override));
            MOCK_METHOD(void, Resume, (), (override));
            MOCK_METHOD(bool, IsPaused, (), (const, override));
            MOCK_METHOD(void, SetSpeedRegulated, (bool regulated), (override));
            MOCK_METHOD(uint64_t, GetEmulatedNanoseconds, (), (const, override));
            MOCK_METHOD(bool, IsSpeedRegulated, (), (const, override));
            MOCK_METHOD(void, SetLowercaseCharset, (bool lowercase), (override));
            MOCK_METHOD(void, SetRomVersion, (Compucolor::RomVersion version), (override));
            MOCK_METHOD(
                std::shared_ptr<Compucolor::Floppy::IFloppyEmulator>,
                GetFloppy,
                (int drive),
                (const, override)
            );
            MOCK_METHOD(
                void,
                SetDebugSink,
                (std::shared_ptr<Compucolor::Debug::IDebugSink> sink),
                (override)
            );
    };

    boost::json::value TestState(const uint64_t nanoseconds)
    {
        boost::json::object state;
        state["emulatedNanoseconds"] = nanoseconds;
        return boost::json::value(std::move(state));
    }

    std::filesystem::path TestPath()
    {
        return std::filesystem::temp_directory_path() / "cc2-processor-state-test.json";
    }
}

TEST(CompucolorEmulatorProcessorTests, StartBlocksUntilEmulatorInitializationCompletes)
{
    auto emulator = std::make_shared<NiceMock<FakeEmulator>>();
    auto entered = std::make_shared<std::promise<void>>();
    auto allow = std::make_shared<std::promise<void>>();

    auto enteredFuture = entered->get_future();
    auto allowFuture = allow->get_future().share();

    ON_CALL(*emulator, Start()).WillByDefault([entered, allowFuture]() mutable {
        entered->set_value();
        allowFuture.wait();
    });

    CompucolorEmulatorProcessor processor(emulator);

    auto returned = std::make_shared<std::promise<void>>();
    auto returnedFuture = returned->get_future();

    std::thread caller([&] {
        processor.Start();
        returned->set_value();
    });

    using namespace std::chrono_literals;
    ASSERT_EQ(enteredFuture.wait_for(2s), std::future_status::ready);
    EXPECT_EQ(returnedFuture.wait_for(50ms), std::future_status::timeout);

    allow->set_value();
    ASSERT_EQ(returnedFuture.wait_for(2s), std::future_status::ready);
    returnedFuture.get();
    caller.join();

    processor.Stop();
}

TEST(CompucolorEmulatorProcessorTests, StopJoinsKeepAliveThreadAndStopsEmulator)
{
    auto emulator = std::make_shared<NiceMock<FakeEmulator>>();
    EXPECT_CALL(*emulator, Stop()).Times(1);

    CompucolorEmulatorProcessor processor(emulator);

    processor.Start();
    processor.Stop();
}

TEST(CompucolorEmulatorProcessorTests, GetStateReturnsSnapshot)
{
    auto emulator = std::make_shared<NiceMock<FakeEmulator>>();
    ON_CALL(*emulator, SaveState()).WillByDefault(
        [this] { return std::make_unique<boost::json::value>(TestState(123)); }
    );

    CompucolorEmulatorProcessor processor(emulator);

    const auto state = processor.GetState();

    EXPECT_EQ(state.as_object().at("emulatedNanoseconds").as_uint64(), 123U);
}

TEST(CompucolorEmulatorProcessorTests, SaveStateWritesToNamedPath)
{
    auto emulator = std::make_shared<NiceMock<FakeEmulator>>();
    ON_CALL(*emulator, SaveState()).WillByDefault(
        [this] { return std::make_unique<boost::json::value>(TestState(456)); }
    );

    CompucolorEmulatorProcessor processor(emulator);
    const auto path = TestPath();
    std::error_code removeError;
    std::filesystem::remove(path, removeError);

    processor.SaveState(path.string());

    ASSERT_TRUE(std::filesystem::exists(path));

    std::ifstream file(path, std::ios::in | std::ios::binary);
    const std::string contents{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    const auto parsed = boost::json::parse(contents);
    EXPECT_EQ(parsed.as_object().at("emulatedNanoseconds").as_int64(), 456);

    std::filesystem::remove(path, removeError);
}

TEST(CompucolorEmulatorProcessorTests, LoadStateReadsFromNamedPath)
{
    auto emulator = std::make_shared<NiceMock<FakeEmulator>>();
    CompucolorEmulatorProcessor processor(emulator);
    const auto path = TestPath();

    {
        std::ofstream file(path, std::ios::out | std::ios::trunc | std::ios::binary);
        file << boost::json::serialize(TestState(789));
    }

    std::shared_ptr<boost::json::value> loaded;
    EXPECT_CALL(*emulator, LoadState(_)).WillOnce(SaveArg<0>(&loaded));

    processor.LoadState(path.string());

    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->as_object().at("emulatedNanoseconds").as_int64(), 789);

    std::error_code removeError;
    std::filesystem::remove(path, removeError);
}
