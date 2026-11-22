#pragma once

#include <string>

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IOutputDevice.h>
#include <Compucolor/Common/IResetable.h>
#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Floppy
{
    // One single-density floppy drive: 41 tracks x 10 sectors x 192 bytes,
    // encoded serially at 76.8 kbit/s. Mount/Eject/Save are called from the UI
    // thread while the emulator thread calls Select/Step/Read/Write.
    class IFloppyEmulator: public Common::IPlugin, public Common::IEmulatable, public Common::IInputDevice, public Common::IOutputDevice, public Common::IResetable {
        public:
            ~IFloppyEmulator() override = default;

            // Drive select, write-enable and raw stepper phase from TMS5501 port 7.
            virtual void Select(bool selected, bool write, uint8_t stepper) = 0;

            // Advances rotation and serial bit timing by the elapsed CPU time.
            virtual void Step(uint64_t elapsedNanoseconds) = 0;

            // True when a decoded byte is waiting for the TMS5501 serial port.
            [[nodiscard]] virtual bool HasData() const = 0;

            [[nodiscard]] virtual bool HasFramingError() const = 0;

            // Throws on malformed images; unlike Mount, these do not report via `error`.
            virtual void ReadFile(std::string filename) = 0;

            virtual void SaveFile(std::string filename) = 0;

            // 3 for the v6.78 ROM (one step per track), 4 for v8.79 (two steps per track).
            virtual void SetStepperPhases(int phases) = 0;

            // Loads a .ccvf image; on failure returns false, sets error and keeps the current disk.
            virtual bool Mount(const std::string& path, std::string& error) = 0;

            // Unmounts the drive and discards its in-memory image.
            virtual void Eject() = 0;

            [[nodiscard]] virtual bool IsMounted() const = 0;

            // Empty when no disk is mounted.
            [[nodiscard]] virtual std::string GetMountedPath() const = 0;

            // True once the emulated machine has written to the disk since mount or last save.
            [[nodiscard]] virtual bool IsModified() const = 0;

            // Writes back to the mounted path.
            virtual bool Save(std::string& error) = 0;

            // Writes to a new path, which becomes the mounted path.
            virtual bool SaveAs(const std::string& path, std::string& error) = 0;
    };
}
