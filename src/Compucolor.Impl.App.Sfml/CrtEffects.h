#pragma once

#include <cstddef>
#include <cstdint>

namespace Compucolor::Impl::App::Sfml
{
    struct CrtEffectSettings {
        bool scanlines = true;
        bool fuzz = true;
        bool vignette = true;
        bool glow = true;

        // Each layer has its own 0-100 strength. The vignette mask must be
        // rebuilt when vignetteIntensity changes.
        int scanlineIntensity = 35;
        int fuzzIntensity = 35;
        int vignetteIntensity = 35;
        int glowIntensity = 35;

        std::uint32_t noiseState = 0x9E3779B9u;
    };

    // Builds a per-pixel brightness multiplier (0-255) for the CRT vignette.
    // Rebuild when the vignette intensity changes.
    void BuildCrtVignetteMask(
        std::uint8_t* mask,
        std::size_t width,
        std::size_t height,
        int vignetteIntensity
    );

    // Applies the enabled CRT layers to a 384x256 RGBA frame before it is
    // uploaded to the display texture. Effects are applied at source
    // resolution so they scale with the emulated screen, not ImGui.
    //
    // source and target must not alias. vignetteMask may be null when the
    // vignette layer is disabled. noiseState is advanced so fuzz shimmers.
    void ApplyCrtEffects(
        const std::uint8_t* source,
        std::uint8_t* target,
        const std::uint8_t* vignetteMask,
        std::size_t width,
        std::size_t height,
        CrtEffectSettings& settings
    );
}
