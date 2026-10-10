
#pragma once

#include <string>

#include <Compucolor/Common/IEmulatable.h>
#include <Compucolor/Common/IInputDevice.h>
#include <Compucolor/Common/IOutputDevice.h>
#include <Compucolor/Common/IResetable.h>
#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Floppy
{
    class IFloppyEmulator: public Common::IPlugin, public Common::IEmulatable, public Common::IInputDevice, public Common::IOutputDevice, public Common::IResetable {
        public:
            ~IFloppyEmulator() override = default;

            virtual void Select(bool selected, bool write, uint8_t stepper) = 0;

            virtual void Step(uint64_t elapsedNanoseconds) = 0;

            [[nodiscard]] virtual bool HasData() const = 0;

            [[nodiscard]] virtual bool HasFramingError() const = 0;

            virtual void ReadFile(std::string filename) = 0;

            virtual void SaveFile(std::string filename) = 0;

            // 3 for the v6.78 ROM (one step per track), 4 for v8.79 (two steps per track).
            virtual void SetStepperPhases(int phases) = 0;

            // Loads a .ccvf image; on failure returns false, sets error and keeps the current disk.
            virtual bool Mount(const std::string& path, std::string& error) = 0;

            virtual void Eject() = 0;

            [[nodiscard]] virtual bool IsMounted() const = 0;

            [[nodiscard]] virtual std::string GetMountedPath() const = 0;

            // True once the emulated machine has written to the disk since mount or last save.
            [[nodiscard]] virtual bool IsModified() const = 0;

            // Writes back to the mounted path.
            virtual bool Save(std::string& error) = 0;

            // Writes to a new path, which becomes the mounted path.
            virtual bool SaveAs(const std::string& path, std::string& error) = 0;
    };
}