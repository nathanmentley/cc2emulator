#include "FloppyEmulator.h"

#include <utility>

using std::make_shared;
using std::shared_ptr;

using Compucolor::Common::IPluginManager;

using Compucolor::Logger::ILogger;
using Compucolor::Floppy::IFloppyEmulator;

using Compucolor::Impl::Floppy::FloppyEmulator;

static constexpr int TRACKS = 40;
static constexpr int SECTORS = 10;
static constexpr int SECTOR_SIZE = 192;

FloppyEmulator::FloppyEmulator(shared_ptr<ILogger> logger):
    _logger(std::move(logger))
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

void FloppyEmulator::Reset()
{
}

uint8_t FloppyEmulator::Read([[maybe_unused]] uint8_t port)
{
    if (!_selected)
        return 0x00;

    uint8_t data = _disk[_bytePos];
    _bytePos = (_bytePos + 1) % _disk.size();
    return data;
}

void FloppyEmulator::Write([[maybe_unused]] uint8_t port, uint8_t data)
{
    if (!_selected)
        return;

    if (_writeEnable) {
        _disk[_bytePos] = data;
        _bytePos = (_bytePos + 1) % _disk.size();
    }
}

void FloppyEmulator::Select(bool selected, bool write, uint8_t stepper)
{
    if (!_selected && selected) {
        _bytePos = _track * SECTORS * SECTOR_SIZE;
    }

    _selected = selected;
    _writeEnable = write;

    StepHead(stepper & 0x03);
}

void FloppyEmulator::ReadFile(std::string filename)
{
}

void FloppyEmulator::SaveFile(std::string filename)
{
}

void FloppyEmulator::StepHead(uint8_t newPhase)
{
    int delta = (newPhase - _stepperPhase) & 0x03;

    if (delta == 1)
        _track++;
    else if (delta == 3)
        _track--;

    if (_track < 0) _track = 0;
    if (_track >= TRACKS) _track = TRACKS - 1;

    _stepperPhase = newPhase;

    _bytePos = _track * SECTORS * SECTOR_SIZE;
}

shared_ptr<IFloppyEmulator> FloppyEmulator::Creator(IPluginManager* manager)
{
    return make_shared<FloppyEmulator>(manager->GetLogger());
}
