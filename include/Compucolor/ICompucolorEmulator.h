#pragma once

#include <memory>

#include <Compucolor/Common/ISystem.h>

#include <Compucolor/Floppy/IFloppyEmulator.h>
#include <Compucolor/Keyboard/CompucolorIIKey.h>
#include <Compucolor/RomVersion.h>

namespace Compucolor
{
    class ICompucolorEmulator: public Common::ISystem {
        public:
            ICompucolorEmulator() = default;

            ~ICompucolorEmulator() override = default;

            virtual void OnKeyUp(Keyboard::CompucolorIIKey key) = 0;

            virtual void OnKeyDown(Keyboard::CompucolorIIKey key) = 0;

            // Warm reset keeps memory; hard reset also reloads the ROM and forces a cold start.
            virtual void Reset(bool hard) = 0;

            virtual void Pause() = 0;

            virtual void Resume() = 0;

            [[nodiscard]] virtual bool IsPaused() const = 0;

            // When false the CPU runs as fast as possible instead of at real time speed.
            virtual void SetSpeedRegulated(bool regulated) = 0;

            [[nodiscard]] virtual bool IsSpeedRegulated() const = 0;

            virtual void SetLowercaseCharset(bool lowercase) = 0;

            // Takes effect through a hard reset.
            virtual void SetRomVersion(RomVersion version) = 0;

            // Drive 0 is A, drive 1 is B; returns null for any other index.
            [[nodiscard]] virtual std::shared_ptr<Floppy::IFloppyEmulator> GetFloppy(int drive) const = 0;
    };
}