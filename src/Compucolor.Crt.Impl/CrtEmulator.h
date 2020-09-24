
#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <thread>

#include <fruit/fruit.h>

#include "Compucolor.Core/IDisplay.h"
#include "Compucolor.Core/IMemory.h"

#include "Compucolor.Crt/ICrtEmulator.h"

#include "Compucolor.Crt.Impl/FontRom.h"

class CrtEmulator: public ICrtEmulator {
    public:
        INJECT(
            CrtEmulator(
                std::shared_ptr<IMemory> memory
            )
        );

        virtual void Start() override;
        virtual void Stop() override;

        virtual void RefreshDisplay() override;

        virtual void SetDisplay(IDisplay* display) override;

    private:
        static int const Columns = 64;
        static int const Rows = 32;
        static int const CharacterWidth = 6;
        static int const CharacterHeight = 8;
        static int const Width = Columns * CharacterWidth;
        static int const Height = Rows * CharacterHeight;

        std::optional<IDisplay*> _display;
        std::shared_ptr<IMemory> _memory;
        std::atomic<bool> _isRunning;
        std::thread _thread;

        void DrawGlyph(Color foreground, Color background, uint8_t glyphData, bool blink, int x, int y);
        bool IsBitSet(int bit, uint8_t data);
        Color GetForegroundColor(uint8_t data);
        Color GetBackgroundColor(uint8_t data);
        Color GetColor(uint8_t data);
};