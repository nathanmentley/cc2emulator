#include "CrtEffects.h"

#include <algorithm>

namespace Compucolor::Impl::App::Sfml
{
    void BuildCrtVignetteMask(
        std::uint8_t* mask,
        const std::size_t width,
        const std::size_t height,
        const int vignetteIntensity
    )
    {
        const float strength = static_cast<float>(std::clamp(vignetteIntensity, 0, 100)) / 100.f;

        for (std::size_t y = 0; y < height; ++y)
        {
            const float ny = (static_cast<float>(y) + 0.5f) / static_cast<float>(height) * 2.f - 1.f;

            for (std::size_t x = 0; x < width; ++x)
            {
                const float nx = (static_cast<float>(x) + 0.5f) / static_cast<float>(width) * 2.f - 1.f;
                const float distanceSquared = nx * nx + ny * ny;

                // 0 at the center, ~1 at the edge midpoint, ~2 in the corners.
                const float falloff = std::clamp((distanceSquared - 0.2f) / 1.3f, 0.f, 1.f);
                const float brightness = 1.f - 0.45f * strength * falloff * falloff;

                mask[y * width + x] = static_cast<std::uint8_t>(brightness * 255.f + 0.5f);
            }
        }
    }

    void ApplyCrtEffects(
        const std::uint8_t* source,
        std::uint8_t* target,
        const std::uint8_t* vignetteMask,
        const std::size_t width,
        const std::size_t height,
        CrtEffectSettings& settings
    )
    {
        // Each layer is scaled independently. At full strength, dark rows lose
        // ~22%, grain is +/-5.5%, and bright pixels bleed into their neighbours.
        const int scanlineStrength = std::clamp(settings.scanlineIntensity, 0, 100);
        const int fuzzStrength = std::clamp(settings.fuzzIntensity, 0, 100);
        const int glowSetting = std::clamp(settings.glowIntensity, 0, 100);

        const int scanlineReduction = settings.scanlines ? (56 * scanlineStrength) / 100 : 0;
        const int fuzzAmplitude = settings.fuzz ? (14 * fuzzStrength) / 100 : 0;
        const unsigned fuzzSpan = static_cast<unsigned>(2 * fuzzAmplitude + 1);
        const int glowStrength = settings.glow ? (40 * glowSetting) / 100 : 0;
        const int glowThreshold = 150 - (20 * glowSetting) / 100;

        for (std::size_t y = 0; y < height; ++y)
        {
            const int rowAdjust = (y % 2 == 1) ? -scanlineReduction : 0;
            std::size_t index = y * width * 4;
            std::size_t pixel = y * width;

            for (std::size_t x = 0; x < width; ++x)
            {
                int noise = 0;

                if (fuzzSpan > 1)
                {
                    settings.noiseState = settings.noiseState * 1664525u + 1013904223u;
                    noise = static_cast<int>((settings.noiseState >> 24) % fuzzSpan) - fuzzAmplitude;
                }

                const int vignette = (settings.vignette && vignetteMask) ? vignetteMask[pixel] : 255;

                for (int channel = 0; channel < 3; ++channel)
                {
                    int value = static_cast<int>(source[index + channel]) + rowAdjust + noise;
                    value = value * vignette / 255;
                    target[index + channel] = static_cast<std::uint8_t>(std::clamp(value, 0, 255));
                }

                target[index + 3] = 0xFF;
                index += 4;
                ++pixel;
            }
        }

        if (glowStrength <= 0)
        {
            return;
        }

        // Phosphor glow: bleed bright source pixels into their neighbours after
        // the base pass so the halo is not darkened by scanlines or vignette.
        const auto addGlow = [target, width, height](const int x, const int y, const int amount) {
            if (x < 0 || y < 0 || x >= static_cast<int>(width) || y >= static_cast<int>(height) || amount <= 0)
            {
                return;
            }

            const std::size_t index = (static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)) * 4;

            for (int channel = 0; channel < 3; ++channel)
            {
                const int value = static_cast<int>(target[index + channel]) + amount;
                target[index + channel] = static_cast<std::uint8_t>(std::clamp(value, 0, 255));
            }
        };

        for (std::size_t y = 0; y < height; ++y)
        {
            for (std::size_t x = 0; x < width; ++x)
            {
                const std::size_t index = (y * width + x) * 4;

                int brightness = source[index];

                for (int channel = 1; channel < 3; ++channel)
                {
                    brightness = std::max(brightness, static_cast<int>(source[index + channel]));
                }

                if (brightness <= glowThreshold)
                {
                    continue;
                }

                const int amount = (brightness - glowThreshold) * glowStrength / 255;
                const int px = static_cast<int>(x);
                const int py = static_cast<int>(y);

                addGlow(px - 1, py, amount);
                addGlow(px + 1, py, amount);
                addGlow(px, py - 1, amount);
                addGlow(px, py + 1, amount);
                addGlow(px - 1, py - 1, amount / 2);
                addGlow(px + 1, py - 1, amount / 2);
                addGlow(px - 1, py + 1, amount / 2);
                addGlow(px + 1, py + 1, amount / 2);
            }
        }
    }
}
