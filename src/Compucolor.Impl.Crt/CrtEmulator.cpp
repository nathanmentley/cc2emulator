#include "CrtEmulator.h"
#include "FontRom.h"

#include <utility>

Compucolor::Impl::Crt::CrtEmulator::CrtEmulator(
    std::shared_ptr<Logger::ILogger> logger,
    std::shared_ptr<Common::IDisplay> display,
    std::shared_ptr<Scheduler::IScheduler> scheduler,
    std::shared_ptr<Memory::IMemory> memory,
    std::shared_ptr<Smc5027::ISmc5027Emulator> smc5027emulator,
    std::shared_ptr<Tms5501::ITms5501Emulator> tms5501Emulator
):
    _logger(std::move(logger)),
    _display(std::move(display)),
    _scheduler(std::move(scheduler)),
    _memory(std::move(memory)),
    _smc5027emulator(std::move(smc5027emulator)),
    _tms5501Emulator(std::move(tms5501Emulator)),
    _phase(0)
{
}

void Compucolor::Impl::Crt::CrtEmulator::Start()
{
    _logger->LogTrace("Starting CrtEmulator");

    _loop = _scheduler->SetupRecurringTask(
        10000000,
        [=, this] {
            RefreshDisplay();

            _phase++;

            if (_phase >= 16)
            {
                _phase = 0;

                _tms5501Emulator->TriggerExternalSensor();
            }

            return 0;
        }
    );
}

void Compucolor::Impl::Crt::CrtEmulator::Stop()
{
    if (_loop)
    {
        _loop->Stop();
        _loop->Join();
        _loop.reset();
    }
}

void Compucolor::Impl::Crt::CrtEmulator::SetLowercaseCharset(const bool lowercase)
{
    _lowercaseCharset.store(lowercase, std::memory_order_relaxed);
}

void Compucolor::Impl::Crt::CrtEmulator::RefreshDisplay() const
{
    _logger->LogTrace(
        "Refreshing Display in CrtEmulator"
    );

    const uint8_t firstRow = _smc5027emulator->FirstDisplayRow();

    for (uint16_t i = 0x7000; i < 0x8000; i += 2)
    {
        // Update video memory
        const uint16_t baseAddress = i - 0x7000;   // 0x6... range to avoid wait for hblank
        // two bytes per char, 64 char/row
        const uint16_t x = (baseAddress >> 1) & 0x3F;
        // 128 bytes per row, 32 rows
        const uint16_t y = (baseAddress >> 7) & 0x1F;

        const uint16_t displayY = (y - firstRow + 32) % 32;

        const uint16_t pair = (i & 0xFFFE);
        const uint8_t ch = _memory->GetByte(pair);
        const uint8_t attrib = _memory->GetByte((pair + 1));  // attribute byte

        const bool blink = IsBitSet(6, attrib);
        const bool plot = IsBitSet(7, attrib);

        DrawGlyph(
            GetForegroundColor(attrib),
            GetBackgroundColor(attrib),
            ch,
            blink,
            plot,
            x,
            displayY
        );
    }

    DrawCursor(firstRow);

    _display->Repaint();
}

void Compucolor::Impl::Crt::CrtEmulator::DrawCursor(const uint8_t firstRow) const
{
    if (IsBlinkOn())
    {
        const uint8_t x = _smc5027emulator->GetCursorX();
        uint8_t y = _smc5027emulator->GetCursorY();
        y = (y - firstRow + 32) % 32;  // scroll adjustment

        if (x <= Columns && y <= Rows)
        {
            DrawGlyph( //TODO: Setup cursor glyph.
                Common::Color::White,
                Common::Color::White,
                0x20,
                false,
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

void Compucolor::Impl::Crt::CrtEmulator::DrawGlyph(
    const Common::Color foreground,
    const Common::Color background,
    const uint8_t glyphData,
    const bool blink,
    const bool plot,
    const int x,
    const int y
) const
{
    const uint16_t xPos = x * CharacterWidth;
    const uint16_t yPos = y * CharacterHeight;

    for(uint16_t row = 0; row < CharacterHeight; row++)
    {
        const uint8_t* uf6_rom = _lowercaseCharset.load(std::memory_order_relaxed) ?
            get_uf6_rom_lowercase() :
            get_uf6_rom();

        const bool isTall = !plot && IsBitSet(7, glyphData);
        const uint8_t ch = glyphData & 0x7F;
        const uint16_t romIndex = (ch * CharacterHeight) + (isTall ? (row/2 + ((y % 2) * 4)): row);
        const uint8_t romData = uf6_rom[romIndex];

        for(uint16_t column = 0; column < CharacterWidth; column++)
        {
            const uint16_t targetX = column + xPos;
            const uint16_t targetY = row + yPos;

            const bool pixelOn = plot ?
                IsBitSet((row / 2) + (column >= 3 ? 4 : 0), glyphData) :
                IsBitSet(7 - column, romData);

            _display->DrawPixel(
                pixelOn ?
                    (blink && IsBlinkOn() ? Common::Color::Black : foreground):
                    background,
                targetX,
                targetY
            );
        }
    }
}

bool Compucolor::Impl::Crt::CrtEmulator::IsBlinkOn() const
{
    return _phase % 2 == 0;
}

Compucolor::Common::Color Compucolor::Impl::Crt::CrtEmulator::GetForegroundColor(uint8_t data)
{
    return GetColor(data & 0x7);
}

Compucolor::Common::Color Compucolor::Impl::Crt::CrtEmulator::GetBackgroundColor(uint8_t data)
{
    return GetColor((data >> 3) & 0x7);
}

Compucolor::Common::Color Compucolor::Impl::Crt::CrtEmulator::GetColor(uint8_t data)
{
    return static_cast<Common::Color>(data);
}

bool Compucolor::Impl::Crt::CrtEmulator::IsBitSet(const int bit, const uint8_t data)
{
    return ((data >> bit) & 0x1) == 1;
}

std::shared_ptr<Compucolor::Crt::ICrtEmulator> Compucolor::Impl::Crt::CrtEmulator::Creator(
    Common::IPluginManager* manager
)
{
    return std::make_shared<CrtEmulator>(
        manager->GetLogger(),
        manager->GetDisplay(),
        manager->GetScheduler(),
        manager->GetMemory(),
        manager->GetSmc5027(),
        manager->GetTms5501()
    );
}
