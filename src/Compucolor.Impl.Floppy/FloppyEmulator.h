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
    // Bit-level floppy drive emulation with a 76.8 kbit serial stream, 41 tracks
    // of 10 sectors x 192 bytes and optional 3/4-phase stepper geometry.
    // A recursive mutex allows the UI to mount/eject while the emulator thread steps.
    class FloppyEmulator : public Compucolor::Floppy::IFloppyEmulator
    {
    public:
        FloppyEmulator(std::shared_ptr<Compucolor::Logger::ILogger> logger);

        void Start() override;
        void Stop() override;
        void Reset() override;

        std::unique_ptr<boost::json::value> SaveState() override;
        void LoadState(std::shared_ptr<boost::json::value>) override;

        // Serial read/write of start, data and stop bits, only while this drive
        // is selected and the operation is enabled by the TMS5501 outport.
        uint8_t Read(uint8_t port) override;
        void Write(uint8_t port, uint8_t data) override;

        // Applies drive select/write enable and steps the head on phase changes.
        void Select(bool selected, bool write, uint8_t stepper) override;

        // Advances rotation and decodes or encodes bytes at the serial bit rate.
        void Step(uint64_t elapsedNanoseconds) override;
        [[nodiscard]] bool HasData() const override;
        [[nodiscard]] bool HasFramingError() const override;

        // .ccvf text parser/writer: "Track N" sections with 64-hex-digit lines
        // (32 bytes each). Parsing uses a scratch image so bad images cannot
        // replace a mounted disk; writes clear the modified flag.
        void ReadFile(std::string filename) override;
        void SaveFile(std::string filename) override;
        void SetStepperPhases(int phases) override;

        // Logical track under the head.
        [[nodiscard]] int GetTrack() const;

        bool Mount(const std::string& path, std::string& error) override;
        void Eject() override;
        [[nodiscard]] bool IsMounted() const override;
        [[nodiscard]] std::string GetMountedPath() const override;
        [[nodiscard]] bool IsModified() const override;
        bool Save(std::string& error) override;
        bool SaveAs(const std::string& path, std::string& error) override;

        static std::shared_ptr<Compucolor::Floppy::IFloppyEmulator>
        Creator(Compucolor::Common::IPluginManager* manager);

    private:
        static constexpr int TRACKS = 41;
        static constexpr int SECTORS = 10;
        static constexpr int SECTOR_SIZE = 192;
        static constexpr int TRACK_SIZE = SECTORS * SECTOR_SIZE;
        static constexpr int DISK_SIZE = TRACKS * TRACK_SIZE;

        std::shared_ptr<Compucolor::Logger::ILogger> _logger;

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
        int _stepperPhases = 4;
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