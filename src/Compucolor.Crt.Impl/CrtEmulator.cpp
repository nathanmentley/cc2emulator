#include <Compucolor.Crt.Impl/CrtEmulator.h>

CrtEmulator::CrtEmulator(
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<IMemory> memory,
    std::shared_ptr<ISmc5027Emulator> smc5027emulator
):
    _display({}),
    _logger(logger),
    _memory(memory),
    _smc5027emulator(smc5027emulator),
    _isRunning(false),
    _phase(0)
{
}

void CrtEmulator::Start()
{
    _logger->LogTrace("Starting %s", NAMEOF_TYPE(CrtEmulator));

    _isRunning = true;
    _thread = std::thread(
        [this]()
        {
            while(_isRunning)
            {
                if(_display.has_value())
                {
                    RefreshDisplay();
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                _phase++;

                if (_phase >= 16)
                {
                    _phase = 0;
                }
            }
        }
    );
}

void CrtEmulator::Stop()
{
    _isRunning = false;
    _thread.join();
}

void CrtEmulator::RefreshDisplay()
{
    if (_display.has_value())
    {
        _logger->LogTrace(
            "Refereshing Display in %s",
            NAMEOF_TYPE(CrtEmulator)
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

        if (IsBlinkOn())
        {
            DrawGlyph( //TODO: Setup cursor glyph.
                Color::White,
                Color::White,
                0x20,
                false,
                _smc5027emulator->GetCursorX(),
                _smc5027emulator->GetCursorY()
            );
        }

        _display.value()->Repaint();
    }
}

void CrtEmulator::SetDisplay(IDisplay* display)
{
    _display = display;
}

void CrtEmulator::DrawGlyph(Color foreground, Color background, uint8_t glyphData, bool blink, int x, int y)
{
    const uint16_t xPos = x * CharacterWidth;
    const uint16_t yPos = y * CharacterHeight;

    for(uint16_t row = 0; row < CharacterHeight; row++)
    {
        const uint8_t* uf6_rom = get_uf6_rom();

        const bool isTall = IsBitSet(7, glyphData);
        const uint8_t ch = glyphData & 0x7F;
        const uint16_t romIndex = (ch * CharacterHeight) + (isTall ? (row/2 + ((y % 2) * 4)): row);
        const uint8_t romData = uf6_rom[romIndex];

        for(uint16_t column = 0; column < CharacterWidth; column++)
        {
            const uint16_t targetX = column + xPos;
            const uint16_t targetY = row + yPos;

            _display.value()->DrawPixel(
                IsBitSet(7 - column, romData) ?
                    (blink && IsBlinkOn() ? Color::Black: foreground):
                    background,
                targetX,
                targetY
            );
        }
    }
}

Color CrtEmulator::GetForegroundColor(uint8_t data)
{
    return GetColor(data & 0x7);
}

Color CrtEmulator::GetBackgroundColor(uint8_t data)
{
    return GetColor((data >> 3) & 0x7);
}

Color CrtEmulator::GetColor(uint8_t data)
{
    return (Color)data;
}

bool CrtEmulator::IsBitSet(int bit, uint8_t data)
{
    return ((data >> bit) & 0x1) == 1;
}

bool CrtEmulator::IsBlinkOn()
{
    return _phase % 2 == 0;
}