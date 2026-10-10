#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <Compucolor/Common/IPluginManager.h>
#include <Compucolor/Floppy/IFloppyEmulator.h>
#include <Compucolor/Logger/ILogger.h>

namespace Compucolor::Impl::Floppy
{
    class FloppyEmulator : public Compucolor::Floppy::IFloppyEmulator
    {
    public:
        FloppyEmulator(std::shared_ptr<Logger::ILogger> logger);

        void Start() override;
        void Stop() override;
        void Reset() override;

        uint8_t Read(uint8_t port) override;
        void Write(uint8_t port, uint8_t data) override;

        void Select(bool selected, bool write, uint8_t stepper) override;
        void Step(uint64_t elapsedNanoseconds) override;
        [[nodiscard]] bool HasData() const override;
        [[nodiscard]] bool HasFramingError() const override;

        void ReadFile(std::string filename) override;
        void SaveFile(std::string filename) override;

        bool Mount(const std::string& path, std::string& error) override;
        void Eject() override;
        [[nodiscard]] bool IsMounted() const override;
        [[nodiscard]] std::string GetMountedPath() const override;
        [[nodiscard]] bool IsModified() const override;
        bool Save(std::string& error) override;
        bool SaveAs(const std::string& path, std::string& error) override;

        static std::shared_ptr<Compucolor::Floppy::IFloppyEmulator>
        Creator(Common::IPluginManager* manager);

    private:
        static constexpr int TRACKS = 41;
        static constexpr int SECTORS = 10;
        static constexpr int SECTOR_SIZE = 192;
        static constexpr int TRACK_SIZE = SECTORS * SECTOR_SIZE;
        static constexpr int DISK_SIZE = TRACKS * TRACK_SIZE;

        std::shared_ptr<Logger::ILogger> _logger;

        // The UI thread mounts and ejects while the emulator thread steps the drive.
        mutable std::recursive_mutex _mutex;
        bool _mounted = false;
        bool _modified = false;
        std::string _mountedPath;
        std::vector<std::string> _labels;

        std::vector<uint8_t> _disk =
            std::vector<uint8_t>(DISK_SIZE, 0x00);

        bool _selected = false;
        bool _writeEnable = false;

        int _track = 0;
        int _stepTrack = 0;
        uint8_t _stepperPhase = 0;
        std::size_t _bitPos = 0;
        uint64_t _bitClockRemainder = 0;
        uint64_t _motorCoastNanoseconds = 0;
        std::optional<std::size_t> _scheduledStartBit;
        std::size_t _bitsUntilRead = 0;
        std::optional<uint8_t> _readData;
        bool _readFramingError = false;

        [[nodiscard]] bool PeekBit(std::size_t position) const;
        [[nodiscard]] std::optional<std::size_t> FindStartBit() const;
        void ScheduleReadByte();
        [[nodiscard]] uint8_t DecodeByte(std::size_t startBit) const;
        void WriteBit(bool bit);
        void StepHead(uint8_t newPhase);
    };
}