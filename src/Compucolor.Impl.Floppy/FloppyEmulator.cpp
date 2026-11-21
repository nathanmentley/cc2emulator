#include "FloppyEmulator.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

using std::make_shared;
using std::shared_ptr;

using Compucolor::Common::IPluginManager;

using Compucolor::Logger::ILogger;
using Compucolor::Floppy::IFloppyEmulator;

using Compucolor::Impl::Floppy::FloppyEmulator;

FloppyEmulator::FloppyEmulator(shared_ptr<ILogger> logger):
    _logger(std::move(logger)),
    _disk(FloppyEmulator::DISK_SIZE, 0x00)
{
    FloppyEmulator::Reset();
}

void FloppyEmulator::Start()
{
    Reset();
}

void FloppyEmulator::Stop()
{
}

std::unique_ptr<boost::json::value> FloppyEmulator::SaveState()
{
    std::lock_guard lock(_mutex);

    boost::json::object state;

    // Disk image
    boost::json::array disk;
    disk.reserve(_disk.size());

    for (const uint8_t byte : _disk)
    {
        disk.push_back(static_cast<uint64_t>(byte));
    }

    state["disk"] = std::move(disk);

    // Disk metadata
    boost::json::array labels;

    for (const auto& label : _labels)
    {
        labels.push_back(boost::json::string(label));
    }

    state["labels"] = std::move(labels);
    state["mounted"] = _mounted;
    state["modified"] = _modified;
    state["mountedPath"] = _mountedPath;

    // Drive state
    state["selected"] = _selected;
    state["writeEnable"] = _writeEnable;

    state["track"] = _track;
    state["stepTrack"] = _stepTrack;
    state["stepperPhases"] = _stepperPhases;
    state["stepperPhase"] = static_cast<uint64_t>(_stepperPhase);

    // Disk rotation and timing
    state["bitPos"] = static_cast<uint64_t>(_bitPos);
    state["bitClockRemainder"] = _bitClockRemainder;
    state["motorCoastNanoseconds"] = _motorCoastNanoseconds;

    // Pending read operation
    state["scheduledStartBit"] = _scheduledStartBit
        ? boost::json::value(static_cast<uint64_t>(*_scheduledStartBit))
        : boost::json::value(nullptr);

    state["bitsUntilRead"] = static_cast<uint64_t>(_bitsUntilRead);

    state["readData"] = _readData
        ? boost::json::value(static_cast<uint64_t>(*_readData))
        : boost::json::value(nullptr);

    state["readFramingError"] = _readFramingError;

    return std::make_unique<boost::json::value>(
        std::move(state)
    );
}

void FloppyEmulator::LoadState(
    std::shared_ptr<boost::json::value> value
)
{
    if (!value || !value->is_object())
    {
        throw std::runtime_error("Invalid FloppyEmulator state");
    }

    const auto& state = value->as_object();

    // Helper for safely reading unsigned integers.
    auto getUnsigned = [](const boost::json::value& value) -> uint64_t
    {
        if (value.is_uint64())
            return value.as_uint64();

        if (value.is_int64() && value.as_int64() >= 0)
            return static_cast<uint64_t>(value.as_int64());

        throw std::runtime_error(
            "Expected non-negative integer in floppy state"
        );
    };

    // Disk image
    const auto& diskJson = state.at("disk").as_array();

    if (diskJson.size() != DISK_SIZE)
    {
        throw std::runtime_error("Invalid floppy disk image size");
    }

    std::vector<uint8_t> disk;
    disk.reserve(DISK_SIZE);

    for (const auto& byte : diskJson)
    {
        const uint64_t number = getUnsigned(byte);

        if (number > 255)
        {
            throw std::runtime_error(
                "Invalid byte value in floppy disk image"
            );
        }

        disk.push_back(static_cast<uint8_t>(number));
    }

    // Labels
    std::vector<std::string> labels;

    for (const auto& label : state.at("labels").as_array())
    {
        if (!label.is_string())
        {
            throw std::runtime_error("Invalid floppy disk label");
        }

        labels.emplace_back(label.as_string());
    }

    // Mount metadata
    const bool mounted = state.at("mounted").as_bool();
    const bool modified = state.at("modified").as_bool();

    const std::string mountedPath(
        state.at("mountedPath").as_string().c_str()
    );

    // Drive state
    const bool selected = state.at("selected").as_bool();
    const bool writeEnable = state.at("writeEnable").as_bool();

    const uint64_t trackValue = getUnsigned(state.at("track"));
    const uint64_t stepTrackValue = getUnsigned(state.at("stepTrack"));
    const uint64_t stepperPhasesValue = getUnsigned(
        state.at("stepperPhases")
    );

    const uint64_t stepperPhaseValue = getUnsigned(
        state.at("stepperPhase")
    );

    // Disk rotation and timing
    const uint64_t bitPosValue = getUnsigned(state.at("bitPos"));

    const uint64_t bitClockRemainder =
        getUnsigned(state.at("bitClockRemainder"));

    const uint64_t motorCoastNanoseconds =
        getUnsigned(state.at("motorCoastNanoseconds"));

    // Pending read operation
    std::optional<std::size_t> scheduledStartBit;

    const auto& scheduledStartBitJson = state.at("scheduledStartBit");

    if (!scheduledStartBitJson.is_null())
    {
        const uint64_t position = getUnsigned(scheduledStartBitJson);

        if (position > std::numeric_limits<std::size_t>::max())
        {
            throw std::runtime_error(
                "Invalid scheduled floppy read position"
            );
        }

        scheduledStartBit = static_cast<std::size_t>(position);
    }

    const uint64_t bitsUntilReadValue =
        getUnsigned(state.at("bitsUntilRead"));

    std::optional<uint8_t> readData;

    const auto& readDataJson = state.at("readData");

    if (!readDataJson.is_null())
    {
        const uint64_t number = getUnsigned(readDataJson);

        if (number > 255)
        {
            throw std::runtime_error("Invalid floppy read data");
        }

        readData = static_cast<uint8_t>(number);
    }

    const bool readFramingError =
        state.at("readFramingError").as_bool();

    // Validate drive state
    constexpr uint64_t BITS_PER_TRACK =
        static_cast<uint64_t>(TRACK_SIZE) * 8;

    if (trackValue >= TRACKS)
    {
        throw std::runtime_error("Invalid floppy track");
    }

    if (stepTrackValue > 80)
    {
        throw std::runtime_error("Invalid floppy step track");
    }

    if (stepperPhasesValue != 3 && stepperPhasesValue != 4)
    {
        throw std::runtime_error("Invalid floppy stepper configuration");
    }

    if (stepperPhaseValue != 0 &&
        stepperPhaseValue != 1 &&
        stepperPhaseValue != 2 &&
        stepperPhaseValue != 4)
    {
        throw std::runtime_error("Invalid floppy stepper phase");
    }

    if (stepperPhasesValue == 3 && stepTrackValue > 40)
    {
        throw std::runtime_error("Invalid three-phase step track");
    }

    const uint64_t expectedTrack = stepperPhasesValue == 3
        ? stepTrackValue
        : stepTrackValue >> 1;

    if (trackValue != expectedTrack)
    {
        throw std::runtime_error("Inconsistent floppy head position");
    }

    if (bitPosValue >= BITS_PER_TRACK)
    {
        throw std::runtime_error("Invalid floppy bit position");
    }

    if (bitClockRemainder >= 1'000'000'000)
    {
        throw std::runtime_error("Invalid floppy bit clock remainder");
    }

    if (motorCoastNanoseconds > 680'000'000)
    {
        throw std::runtime_error("Invalid floppy motor coast duration");
    }

    if (scheduledStartBit && *scheduledStartBit >= BITS_PER_TRACK)
    {
        throw std::runtime_error("Invalid scheduled floppy read position");
    }

    if (bitsUntilReadValue > BITS_PER_TRACK + 10)
    {
        throw std::runtime_error("Invalid floppy read countdown");
    }

    if (bitsUntilReadValue > std::numeric_limits<std::size_t>::max())
    {
        throw std::runtime_error("Invalid floppy read countdown");
    }

    // Apply restored state.
    // Everything above this point happens before modifying the drive.
    std::lock_guard lock(_mutex);

    _disk = std::move(disk);
    _labels = std::move(labels);

    _mounted = mounted;
    _modified = modified;
    _mountedPath = mountedPath;

    _selected = selected;
    _writeEnable = writeEnable;

    _track = static_cast<int>(trackValue);
    _stepTrack = static_cast<int>(stepTrackValue);
    _stepperPhases = static_cast<int>(stepperPhasesValue);
    _stepperPhase = static_cast<uint8_t>(stepperPhaseValue);

    _bitPos = static_cast<std::size_t>(bitPosValue);
    _bitClockRemainder = bitClockRemainder;
    _motorCoastNanoseconds = motorCoastNanoseconds;

    _scheduledStartBit = scheduledStartBit;
    _bitsUntilRead = static_cast<std::size_t>(bitsUntilReadValue);

    _readData = readData;
    _readFramingError = readFramingError;
}

void FloppyEmulator::Reset()
{
    std::lock_guard lock(_mutex);

    _selected = false;
    _writeEnable = false;
    _track = 0;
    _stepTrack = 0;
    _stepperPhase = 0;
    _bitPos = 0;
    _bitClockRemainder = 0;
    _motorCoastNanoseconds = 0;
    _scheduledStartBit.reset();
    _bitsUntilRead = 0;
    _readData.reset();
    _readFramingError = false;
}

uint8_t FloppyEmulator::Read([[maybe_unused]] uint8_t port)
{
    std::lock_guard lock(_mutex);

    if (!_selected || _writeEnable)
        return 0x00;

    if (!_readData)
        return 0x00;

    const uint8_t data = *_readData;
    _readData.reset();
    ScheduleReadByte();
    return data;
}

void FloppyEmulator::Write(
    [[maybe_unused]] uint8_t port,
    uint8_t data
)
{
    std::lock_guard lock(_mutex);

    if (!_selected || !_writeEnable)
        return;

    WriteBit(false);
    for (std::size_t bit = 0; bit < 8; ++bit)
        WriteBit((data & (1U << bit)) != 0);
    WriteBit(true);
}

bool FloppyEmulator::HasData() const
{
    std::lock_guard lock(_mutex);

    return _selected && !_writeEnable && _readData.has_value();
}

bool FloppyEmulator::HasFramingError() const
{
    std::lock_guard lock(_mutex);

    return _readFramingError;
}

void FloppyEmulator::Select(
    bool selected,
    bool write,
    uint8_t stepper
)
{
    std::lock_guard lock(_mutex);

    if (!selected)
    {
        if (_selected)
        {
            _motorCoastNanoseconds = 680'000'000;
            _scheduledStartBit.reset();
            _bitsUntilRead = 0;
            _readData.reset();
        }
        _selected = false;
        _writeEnable = false;
        return;
    }

    _selected = true;
    _motorCoastNanoseconds = 0;
    _writeEnable = write;

    const int oldTrack = _track;
    StepHead(stepper & 0x07);
    if (_track != oldTrack)
    {
        _scheduledStartBit.reset();
        _bitsUntilRead = 0;
        _readData.reset();
    }

    if (_writeEnable)
    {
        _scheduledStartBit.reset();
        _bitsUntilRead = 0;
    }
    else
    {
        ScheduleReadByte();
    }
}

void FloppyEmulator::Step(uint64_t elapsedNanoseconds)
{
    std::lock_guard lock(_mutex);

    if (!_selected && _motorCoastNanoseconds == 0)
        return;

    constexpr uint64_t NANOSECONDS_PER_SECOND = 1'000'000'000;
    constexpr uint64_t DISK_BIT_RATE = 76'800;
    const uint64_t activeNanoseconds = _selected
        ? elapsedNanoseconds
        : std::min(elapsedNanoseconds, _motorCoastNanoseconds);
    if (!_selected)
        _motorCoastNanoseconds -= activeNanoseconds;

    _bitClockRemainder += DISK_BIT_RATE * activeNanoseconds;
    const std::size_t bitsToAdvance = static_cast<std::size_t>(
        _bitClockRemainder / NANOSECONDS_PER_SECOND
    );
    _bitClockRemainder %= NANOSECONDS_PER_SECOND;

    if (_writeEnable)
    {
        for (std::size_t bit = 0; bit < bitsToAdvance; ++bit)
            WriteBit(true);
        return;
    }

    const std::size_t bitsPerTrack =
        static_cast<std::size_t>(TRACK_SIZE) * 8;
    _bitPos = (_bitPos + bitsToAdvance) % bitsPerTrack;

    if (!_selected)
        return;

    if (_scheduledStartBit)
    {
        if (bitsToAdvance >= _bitsUntilRead)
        {
            const std::size_t overshoot = bitsToAdvance - _bitsUntilRead;
            _readData = DecodeByte(*_scheduledStartBit);
            _readFramingError = !PeekBit(*_scheduledStartBit + 9);
            _bitPos = (*_scheduledStartBit + 10) % bitsPerTrack;
            _scheduledStartBit.reset();
            _bitsUntilRead = 0;

            ScheduleReadByte();
            if (_scheduledStartBit)
                _bitsUntilRead -= overshoot;
            _bitPos = (_bitPos + overshoot) % bitsPerTrack;
        }
        else
        {
            _bitsUntilRead -= bitsToAdvance;
        }
    }

}

bool FloppyEmulator::PeekBit(std::size_t position) const
{
    const std::size_t bitsPerTrack =
        static_cast<std::size_t>(TRACK_SIZE) * 8;
    position %= bitsPerTrack;

    const std::size_t trackOffset =
        static_cast<std::size_t>(_track) * TRACK_SIZE;
    const uint8_t byte = _disk[trackOffset + position / 8];
    return (byte & (1U << (position & 0x07))) != 0;
}

std::optional<std::size_t> FloppyEmulator::FindStartBit() const
{
    const std::size_t bitsPerTrack =
        static_cast<std::size_t>(TRACK_SIZE) * 8;
    bool previousBit = PeekBit((_bitPos + bitsPerTrack - 1) % bitsPerTrack);

    for (std::size_t offset = 0; offset < bitsPerTrack; ++offset)
    {
        const std::size_t position = (_bitPos + offset) % bitsPerTrack;
        const bool currentBit = PeekBit(position);
        if (previousBit && !currentBit)
            return position;
        previousBit = currentBit;
    }

    return std::nullopt;
}

void FloppyEmulator::ScheduleReadByte()
{
    if (!_selected || _writeEnable || _scheduledStartBit || !_mounted)
        return;

    const auto startBit = FindStartBit();
    if (!startBit)
        return;

    const std::size_t bitsPerTrack =
        static_cast<std::size_t>(TRACK_SIZE) * 8;
    const std::size_t distance =
        (*startBit + bitsPerTrack - _bitPos) % bitsPerTrack;
    _scheduledStartBit = *startBit;
    _bitsUntilRead = distance + 10;
}

uint8_t FloppyEmulator::DecodeByte(std::size_t startBit) const
{
    uint8_t data = 0;
    for (std::size_t bit = 0; bit < 8; ++bit)
    {
        if (PeekBit(startBit + bit + 1))
            data |= static_cast<uint8_t>(1U << bit);
    }
    return data;
}

void FloppyEmulator::WriteBit(bool bit)
{
    const std::size_t trackOffset =
        static_cast<std::size_t>(_track) * TRACK_SIZE;
    const std::size_t byteOffset = trackOffset + _bitPos / 8;
    const uint8_t mask = static_cast<uint8_t>(1U << (_bitPos & 0x07));

    if (_mounted)
    {
        const uint8_t before = _disk[byteOffset];

        if (bit)
            _disk[byteOffset] |= mask;
        else
            _disk[byteOffset] &= static_cast<uint8_t>(~mask);

        if (_disk[byteOffset] != before)
            _modified = true;
    }

    _bitPos = (_bitPos + 1) % (TRACK_SIZE * 8);
}

void FloppyEmulator::ReadFile(std::string filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        throw std::runtime_error(
            "Unable to open floppy image: " + filename
        );
    }

    std::string line;
    int currentTrack = -1;
    std::size_t trackOffset = 0;

    // Parse into a scratch image so a bad file leaves the current disk untouched.
    std::vector<uint8_t> disk(DISK_SIZE, 0x00);
    std::vector<std::string> labels;

    while (std::getline(file, line))
    {
        // Handle Windows CRLF line endings.
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        // Ignore blank lines.
        if (line.empty())
            continue;

        // Track header.
        if (line.rfind("Track ", 0) == 0)
        {
            std::istringstream stream(line.substr(6));

            int imageTrack = -1;
            stream >> imageTrack;

            if (stream.fail())
            {
                throw std::runtime_error(
                    "Invalid track header: " + line
                );
            }

            if (imageTrack < 0 || imageTrack >= TRACKS)
            {
                throw std::runtime_error(
                    "Track outside floppy geometry: " +
                    std::to_string(imageTrack)
                );
            }

            currentTrack = imageTrack;
            trackOffset = 0;

            continue;
        }

        // Ignore metadata before the first track.
        if (currentTrack < 0)
        {
            if (line.rfind("Label ", 0) == 0)
                labels.push_back(line);

            continue;
        }

        // Each data line contains 32 bytes represented
        // as 64 hexadecimal characters.
        if (line.size() != 64)
        {
            throw std::runtime_error(
                "Invalid floppy data line length on track " +
                std::to_string(currentTrack) +
                ": expected 64 hex characters, got " +
                std::to_string(line.size())
            );
        }

        if (trackOffset + 32 > TRACK_SIZE)
        {
            throw std::runtime_error(
                "Too much data on track " +
                std::to_string(currentTrack)
            );
        }

        const std::size_t diskOffset =
            static_cast<std::size_t>(currentTrack) * TRACK_SIZE +
            trackOffset;

        for (std::size_t i = 0; i < 32; ++i)
        {
            const std::string byteString =
                line.substr(i * 2, 2);

            unsigned int value = 0;

            std::istringstream stream(byteString);
            stream >> std::hex >> value;

            if (stream.fail())
            {
                throw std::runtime_error(
                    "Invalid hexadecimal data on track " +
                    std::to_string(currentTrack)
                );
            }

            disk[diskOffset + i] =
                static_cast<uint8_t>(value);
        }

        trackOffset += 32;
    }

    std::lock_guard lock(_mutex);

    _disk = std::move(disk);
    _labels = std::move(labels);
    _mounted = true;
    _modified = false;
    _mountedPath = filename;

    // Put the virtual drive back at track zero.
    _track = 0;
    _stepTrack = 0;
    _stepperPhase = 0;
    _bitPos = 0;
    _bitClockRemainder = 0;
    _motorCoastNanoseconds = 0;
    _scheduledStartBit.reset();
    _bitsUntilRead = 0;
    _readData.reset();
    _readFramingError = false;
}

void FloppyEmulator::SaveFile(std::string filename)
{
    std::lock_guard lock(_mutex);

    std::ofstream file(filename);

    if (!file)
    {
        throw std::runtime_error(
            "Unable to open floppy image for writing: " +
            filename
        );
    }

    file << "Compucolor Virtual Floppy Disk Image\n";

    for (const std::string& label : _labels)
        file << label << '\n';

    for (int track = 0; track < TRACKS; ++track)
    {
        file << std::dec << "Track " << track << '\n';

        const std::size_t trackOffset =
            static_cast<std::size_t>(track) * TRACK_SIZE;

        for (std::size_t offset = 0;
             offset < TRACK_SIZE;
             offset += 32)
        {
            for (std::size_t i = 0; i < 32; ++i)
            {
                file
                    << std::hex
                    << std::setw(2)
                    << std::setfill('0')
                    << static_cast<unsigned int>(
                        _disk[trackOffset + offset + i]
                    );
            }

            file << '\n';
        }
    }

    file.flush();

    if (!file)
    {
        throw std::runtime_error("Failed writing floppy image: " + filename);
    }

    _mountedPath = filename;
    _modified = false;
}

bool FloppyEmulator::Mount(const std::string& path, std::string& error)
{
    try
    {
        ReadFile(path);
    }
    catch (const std::exception& exception)
    {
        error = exception.what();
        return false;
    }

    return true;
}

void FloppyEmulator::Eject()
{
    std::lock_guard lock(_mutex);

    std::fill(_disk.begin(), _disk.end(), 0x00);
    _labels.clear();
    _mounted = false;
    _modified = false;
    _mountedPath.clear();

    _bitPos = 0;
    _bitClockRemainder = 0;
    _scheduledStartBit.reset();
    _bitsUntilRead = 0;
    _readData.reset();
    _readFramingError = false;
}

bool FloppyEmulator::IsMounted() const
{
    std::lock_guard lock(_mutex);

    return _mounted;
}

std::string FloppyEmulator::GetMountedPath() const
{
    std::lock_guard lock(_mutex);

    return _mountedPath;
}

bool FloppyEmulator::IsModified() const
{
    std::lock_guard lock(_mutex);

    return _modified;
}

bool FloppyEmulator::Save(std::string& error)
{
    std::string path;

    {
        std::lock_guard lock(_mutex);

        if (!_mounted || _mountedPath.empty())
        {
            error = "No disk is mounted";
            return false;
        }

        path = _mountedPath;
    }

    return SaveAs(path, error);
}

bool FloppyEmulator::SaveAs(const std::string& path, std::string& error)
{
    try
    {
        SaveFile(path);
    }
    catch (const std::exception& exception)
    {
        error = exception.what();
        return false;
    }

    return true;
}

void FloppyEmulator::StepHead(uint8_t newPhase)
{
    // A 3 phase motor has no phase 0 state, so that input leaves the head where it is.
    const bool threePhase = _stepperPhases == 3;
    const bool validPhase = newPhase == 1 || newPhase == 2 || newPhase == 4 || (!threePhase && newPhase == 0);

    if (!validPhase)
        return;

    const bool stepIn = threePhase
        ? (_stepperPhase == 1 && newPhase == 2) ||
          (_stepperPhase == 2 && newPhase == 4) ||
          (_stepperPhase == 4 && newPhase == 1)
        : (_stepperPhase == 1 && newPhase == 2) ||
          (_stepperPhase == 2 && newPhase == 4) ||
          (_stepperPhase == 4 && newPhase == 0) ||
          (_stepperPhase == 0 && newPhase == 1);
    const bool stepOut = threePhase
        ? (_stepperPhase == 1 && newPhase == 4) ||
          (_stepperPhase == 2 && newPhase == 1) ||
          (_stepperPhase == 4 && newPhase == 2)
        : (_stepperPhase == 1 && newPhase == 0) ||
          (_stepperPhase == 2 && newPhase == 1) ||
          (_stepperPhase == 4 && newPhase == 2) ||
          (_stepperPhase == 0 && newPhase == 4);

    const int maxStepTrack = threePhase ? 40 : 80;

    if (stepIn)
        _stepTrack = std::min(_stepTrack + 1, maxStepTrack);
    else if (stepOut)
        _stepTrack = std::max(_stepTrack - 1, 0);

    // A 3 phase motor moves one track per step; a 4 phase motor needs two.
    _track = threePhase ? _stepTrack : _stepTrack >> 1;

    _stepperPhase = newPhase;
}

void FloppyEmulator::SetStepperPhases(const int phases)
{
    std::lock_guard lock(_mutex);

    _stepperPhases = phases == 3 ? 3 : 4;

    // The step count means something different now, so park the head.
    _stepTrack = 0;
    _track = 0;
    _stepperPhase = 0;
    _scheduledStartBit.reset();
    _bitsUntilRead = 0;
    _readData.reset();
}

int FloppyEmulator::GetTrack() const
{
    std::lock_guard lock(_mutex);

    return _track;
}

shared_ptr<IFloppyEmulator> FloppyEmulator::Creator(
    IPluginManager* manager
)
{
    return make_shared<FloppyEmulator>(
        manager->GetLogger()
    );
}