#include <array>
#include <cstdint>

#include <gtest/gtest.h>

#include "CrtEffects.h"

using Compucolor::Impl::App::Sfml::ApplyCrtEffects;
using Compucolor::Impl::App::Sfml::BuildCrtVignetteMask;
using Compucolor::Impl::App::Sfml::CrtEffectSettings;

namespace
{
    constexpr std::size_t Width = 4;
    constexpr std::size_t Height = 4;
    constexpr std::size_t FrameSize = Width * Height * 4;
    constexpr std::size_t MaskSize = Width * Height;

    using Frame = std::array<std::uint8_t, FrameSize>;
    using Mask = std::array<std::uint8_t, MaskSize>;

    Frame FilledFrame(const std::uint8_t value)
    {
        Frame frame{};
        frame.fill(value);

        for (std::size_t alpha = 3; alpha < FrameSize; alpha += 4)
        {
            frame[alpha] = 0xFF;
        }

        return frame;
    }

    std::size_t PixelIndex(const std::size_t x, const std::size_t y)
    {
        return (y * Width + x) * 4;
    }

    int RowAverage(const Frame& frame, const std::size_t row)
    {
        int total = 0;

        for (std::size_t x = 0; x < Width; ++x)
        {
            total += frame[PixelIndex(x, row)];
        }

        return total / static_cast<int>(Width);
    }

    void SetIntensities(CrtEffectSettings& settings, const int value)
    {
        settings.scanlineIntensity = value;
        settings.fuzzIntensity = value;
        settings.vignetteIntensity = value;
        settings.glowIntensity = value;
    }

    void Apply(
        const Frame& source,
        Frame& target,
        CrtEffectSettings settings,
        const Mask* mask = nullptr
    )
    {
        ApplyCrtEffects(
            source.data(),
            target.data(),
            mask ? mask->data() : nullptr,
            Width,
            Height,
            settings
        );
    }
}

TEST(CrtEffects, IntensityZeroCopiesTheSource) {
    //Arrange
    const Frame source = FilledFrame(128);
    Frame target{};
    Mask mask{};
    CrtEffectSettings settings;
    SetIntensities(settings, 0);
    settings.noiseState = 1;
    BuildCrtVignetteMask(mask.data(), Width, Height, settings.vignetteIntensity);

    //Act
    Apply(source, target, settings, &mask);

    //Assert
    EXPECT_EQ(target, source);
    EXPECT_EQ(settings.noiseState, 1u);
}

TEST(CrtEffects, SameSeedProducesTheSameFrame) {
    //Arrange
    const Frame source = FilledFrame(160);
    Frame first{};
    Frame second{};
    Mask mask{};
    CrtEffectSettings firstSettings;
    CrtEffectSettings secondSettings;
    SetIntensities(firstSettings, 75);
    firstSettings.noiseState = 7;
    secondSettings = firstSettings;
    BuildCrtVignetteMask(mask.data(), Width, Height, firstSettings.vignetteIntensity);

    //Act
    Apply(source, first, firstSettings, &mask);
    Apply(source, second, secondSettings, &mask);

    //Assert
    EXPECT_EQ(first, second);
    EXPECT_EQ(firstSettings.noiseState, secondSettings.noiseState);
}

TEST(CrtEffects, DifferentSeedsProduceDifferentFuzz) {
    //Arrange
    const Frame source = FilledFrame(160);
    Frame first{};
    Frame second{};
    CrtEffectSettings firstSettings;
    CrtEffectSettings secondSettings;

    firstSettings.scanlines = false;
    firstSettings.vignette = false;
    firstSettings.glow = false;
    SetIntensities(firstSettings, 100);
    firstSettings.noiseState = 11;

    secondSettings = firstSettings;
    secondSettings.noiseState = 999;

    //Act
    Apply(source, first, firstSettings);
    Apply(source, second, secondSettings);

    //Assert
    EXPECT_NE(first, second);
}

TEST(CrtEffects, ScanlinesDarkenAlternateRows) {
    //Arrange
    const Frame source = FilledFrame(200);
    Frame target{};
    CrtEffectSettings settings;
    settings.fuzz = false;
    settings.vignette = false;
    settings.glow = false;
    SetIntensities(settings, 100);

    //Act
    Apply(source, target, settings);

    //Assert
    EXPECT_LT(RowAverage(target, 1), RowAverage(target, 0) - 20);
    EXPECT_LT(RowAverage(target, 3), RowAverage(target, 2) - 20);
}

TEST(CrtEffects, LayerIntensitiesAreIndependent) {
    //Arrange
    const Frame source = FilledFrame(200);
    Frame strong{};
    Frame weak{};
    CrtEffectSettings strongSettings;
    CrtEffectSettings weakSettings;

    strongSettings.fuzz = false;
    strongSettings.vignette = false;
    strongSettings.glow = false;
    SetIntensities(strongSettings, 0);
    strongSettings.scanlineIntensity = 100;

    weakSettings = strongSettings;
    weakSettings.scanlineIntensity = 0;

    //Act
    Apply(source, strong, strongSettings);
    Apply(source, weak, weakSettings);

    //Assert
    EXPECT_EQ(RowAverage(strong, 0), RowAverage(weak, 0));
    EXPECT_LT(RowAverage(strong, 1), RowAverage(weak, 1) - 20);
}

TEST(CrtEffects, VignetteDarkensCorners) {
    //Arrange
    const Frame source = FilledFrame(200);
    Frame target{};
    Mask mask{};
    CrtEffectSettings settings;
    settings.scanlines = false;
    settings.fuzz = false;
    settings.glow = false;
    SetIntensities(settings, 100);
    BuildCrtVignetteMask(mask.data(), Width, Height, settings.vignetteIntensity);

    //Act
    Apply(source, target, settings, &mask);

    //Assert
    EXPECT_LT(target[PixelIndex(0, 0)], target[PixelIndex(1, 1)] - 20);
}

TEST(CrtEffects, GlowBrightensNeighbouringPixels) {
    //Arrange
    Frame source = FilledFrame(20);
    Frame target{};
    CrtEffectSettings settings;
    settings.scanlines = false;
    settings.fuzz = false;
    settings.vignette = false;
    SetIntensities(settings, 100);
    source[PixelIndex(1, 1)] = 255;
    source[PixelIndex(1, 1) + 1] = 255;
    source[PixelIndex(1, 1) + 2] = 255;

    //Act
    Apply(source, target, settings);

    //Assert
    EXPECT_GT(target[PixelIndex(0, 1)], 20);
    EXPECT_GT(target[PixelIndex(1, 0)], 20);
    EXPECT_GT(target[PixelIndex(0, 0)], 20);
}

TEST(CrtEffects, DisabledLayersAreSkipped) {
    //Arrange
    const Frame source = FilledFrame(180);
    Frame target{};
    Mask mask{};
    CrtEffectSettings settings;
    settings.scanlines = false;
    settings.fuzz = false;
    settings.vignette = false;
    settings.glow = false;
    SetIntensities(settings, 100);
    BuildCrtVignetteMask(mask.data(), Width, Height, settings.vignetteIntensity);

    //Act
    Apply(source, target, settings, &mask);

    //Assert
    EXPECT_EQ(target, source);
}

TEST(CrtEffects, AlphaRemainsOpaque) {
    //Arrange
    const Frame source = FilledFrame(90);
    Frame target{};
    Mask mask{};
    CrtEffectSettings settings;
    SetIntensities(settings, 100);
    BuildCrtVignetteMask(mask.data(), Width, Height, settings.vignetteIntensity);

    //Act
    Apply(source, target, settings, &mask);

    //Assert
    for (std::size_t alpha = 3; alpha < FrameSize; alpha += 4)
    {
        EXPECT_EQ(target[alpha], 0xFF);
    }
}
