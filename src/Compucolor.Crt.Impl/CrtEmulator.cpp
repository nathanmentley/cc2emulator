#include "CrtEmulator.h"

CrtEmulator::CrtEmulator(
    std::shared_ptr<IMemory> memory
):
    _display({}),
    _memory(memory),
    _isRunning(false)
{

}

void CrtEmulator::Start()
{
    _isRunning = true;
    _thread = std::thread(
        [this]() {
            while(_isRunning) {
                if(_display.has_value())
                {
                    RefreshDisplay();
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

            if (true) {
                DrawGlyph(
                    GetForegroundColor(attrib),
                    GetBackgroundColor(attrib),
                    ch,
                    blink,
                    x,
                    y
                );
            }
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

            if (IsBitSet(7 - column, romData))
            {
                _display.value()->DrawPixel(foreground, targetX, targetY);
            }
            else
            {
                _display.value()->DrawPixel(background, targetX, targetY);
            }
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
    switch(data)
    {
        case 0:
            return Black;
        case 1:
            return Red;
        case 2:
            return Green;
        case 3:
            return Yellow;
        case 4:
            return Blue;
        case 5:
            return Purple;
        case 6:
            return Teal;
        case 7:
            return White;
        default:
            return White;
    }
}

bool CrtEmulator::IsBitSet(int bit, uint8_t data)
{
    return ((data >> bit) & 0x1) == 1;
}