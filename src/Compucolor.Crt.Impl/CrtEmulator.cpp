#include "CrtEmulator.h"

Compucolor::Crt::Impl::CrtEmulator::CrtEmulator(
    std::shared_ptr<Logger::ILogger> logger,
    std::shared_ptr<Common::IDisplay> display,
    std::shared_ptr<Scheduler::IScheduler> scheduler,
    std::shared_ptr<Memory::IMemory> memory,
    std::shared_ptr<Smc5027::ISmc5027Emulator> smc5027emulator
):
    _logger(logger),
    _display(display),
    _scheduler(scheduler),
    _memory(memory),
    _smc5027emulator(smc5027emulator),
    _phase(0)
{
}

void Compucolor::Crt::Impl::CrtEmulator::Start()
{
    _logger->LogTrace("Starting CrtEmulator");

    _loop = _scheduler->SetupReoccuringTask(
        10000000,
        [=, this] {
            RefreshDisplay();

            _phase++;

            if (_phase >= 16)
            {
                _phase = 0;
            }

            return 0;
        }
    );
}

void Compucolor::Crt::Impl::CrtEmulator::Stop()
{
}

void Compucolor::Crt::Impl::CrtEmulator::RefreshDisplay()
{
    _logger->LogTrace(
        "Refereshing Display in CrtEmulator"
    );

    for (uint16_t i = 0x7000; i < 0x8000; i += 2)
    {
        // Update video memory
        uint16_t baseAddress = i - 0x7000;   // 0x6... range to avoid wait for hblank
        // two bytes per char, 64 char/row
        uint16_t x = (baseAddress >> 1) & 0x3F;
        // 128 bytes per row, 32 rows
        uint16_t y = (baseAddress >> 7) & 0x1F;

        uint16_t pair = (i & 0xFFFE);
        uint8_t ch = _memory->GetByte(pair);
        uint8_t attrib = _memory->GetByte((pair + 1));  // attribute byte

        bool blink = IsBitSet(7, attrib);//((attrib & 0x40) != 0);

        DrawGlyph(
            GetForegroundColor(attrib),
            GetBackgroundColor(attrib),
            ch,
            blink,
            x,
            y
        );
    }

    DrawCursor();

    _display->Repaint();
}

void Compucolor::Crt::Impl::CrtEmulator::DrawCursor()
{
    if (IsBlinkOn())
    {
        uint8_t x = _smc5027emulator->GetCursorX();
        uint8_t y = _smc5027emulator->GetCursorY();

        if (x <= CrtEmulator::Columns && y <= CrtEmulator::Rows)
        {
            DrawGlyph( //TODO: Setup cursor glyph.
                Compucolor::Common::Color::White,
                Compucolor::Common::Color::White,
                0x20,
                false,
                x,
                y
            );
        }
        else
        {
            _logger->LogDebug(
                "Not Drawing Cursor in CrtEmulator because the cursor is off the screen at x: %d y: %d",
                x,
                y
            );
        }
        
    }
}

void Compucolor::Crt::Impl::CrtEmulator::DrawGlyph(Compucolor::Common::Color foreground, Compucolor::Common::Color background, uint8_t glyphData, bool blink, int x, int y)
{
    const uint16_t xPos = x * CharacterWidth;
    const uint16_t yPos = y * CharacterHeight;

    for(uint16_t row = 0; row < CharacterHeight; row++)
    {
        const uint8_t* uf6_rom = Compucolor::Crt::Impl::get_uf6_rom();

        const bool isTall = IsBitSet(7, glyphData);
        const uint8_t ch = glyphData & 0x7F;
        const uint16_t romIndex = (ch * CharacterHeight) + (isTall ? (row/2 + ((y % 2) * 4)): row);
        const uint8_t romData = uf6_rom[romIndex];

        for(uint16_t column = 0; column < CharacterWidth; column++)
        {
            const uint16_t targetX = column + xPos;
            const uint16_t targetY = row + yPos;

            _display->DrawPixel(
                IsBitSet(7 - column, romData) ?
                    (blink && IsBlinkOn() ? Compucolor::Common::Color::Black: foreground):
                    background,
                targetX,
                targetY
            );
        }
    }
}

bool Compucolor::Crt::Impl::CrtEmulator::IsBlinkOn()
{
    return _phase % 2 == 0;
}

Compucolor::Common::Color Compucolor::Crt::Impl::CrtEmulator::GetForegroundColor(uint8_t data)
{
    return GetColor(data & 0x7);
}

Compucolor::Common::Color Compucolor::Crt::Impl::CrtEmulator::GetBackgroundColor(uint8_t data)
{
    return GetColor((data >> 3) & 0x7);
}

Compucolor::Common::Color Compucolor::Crt::Impl::CrtEmulator::GetColor(uint8_t data)
{
    return (Compucolor::Common::Color)data;
}

bool Compucolor::Crt::Impl::CrtEmulator::IsBitSet(int bit, uint8_t data)
{
    return ((data >> bit) & 0x1) == 1;
}