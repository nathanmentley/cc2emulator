
#pragma once

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
    };
}