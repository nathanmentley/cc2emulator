#pragma once

#include <memory>

#include <Compucolor/Common/ISystem.h>

#include <Compucolor/Floppy/IFloppyEmulator.h>
#include <Compucolor/Keyboard/CompucolorIIKey.h>
#include <Compucolor/RomVersion.h>

namespace Compucolor
{
    // Contract for the whole Compucolor II machine, used by the host application.
    class ICompucolorEmulator: public Common::ISystem {
        public:
            ICompucolorEmulator() = default;

            ~ICompucolorEmulator() override = default;

            // Key events are forwarded to the emulated keyboard matrix.
            virtual void OnKeyUp(Keyboard::CompucolorIIKey key) = 0;

            virtual void OnKeyDown(Keyboard::CompucolorIIKey key) = 0;

            // Warm reset keeps memory; hard reset also reloads the ROM and forces a cold start.
            // The request is applied on the next machine tick, never during a CPU step.
            virtual void Reset(bool hard) = 0;

            // Pausing stops CPU stepping; pending resets are still processed.
            virtual void Pause() = 0;

            virtual void Resume() = 0;

            [[nodiscard]] virtual bool IsPaused() const = 0;

            // When false the CPU runs in bounded 4096-step bursts per 1 ms tick
            // instead of being paced to roughly 2 MHz.
            virtual void SetSpeedRegulated(bool regulated) = 0;

            [[nodiscard]] virtual bool IsSpeedRegulated() const = 0;

            // Selects the standard or lowercase UF6 font used by the CRT.
            virtual void SetLowercaseCharset(bool lowercase) = 0;

            // Takes effect through a hard reset, including floppy stepper geometry.
            virtual void SetRomVersion(RomVersion version) = 0;

            // Drive 0 is A, drive 1 is B; returns null for any other index.
            [[nodiscard]] virtual std::shared_ptr<Floppy::IFloppyEmulator> GetFloppy(int drive) const = 0;
    };
}