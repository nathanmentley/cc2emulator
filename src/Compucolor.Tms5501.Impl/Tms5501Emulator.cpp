#include "Tms5501Emulator.h"

Tms5501Emulator::Tms5501Emulator(
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<IIntel8080Emulator> intel8080,
    std::shared_ptr<IKeyboardEmulator> keyboard,
    std::shared_ptr<IFloppyEmulator> floppy1,
    std::shared_ptr<IFloppyEmulator> floppy2
):
    _logger(logger),
    _context(std::unique_ptr<Tms5501EmulatorContext>(new Tms5501EmulatorContext())),
    _intel8080(intel8080),
    _isRunning(false),
    _keyboard(keyboard),
    _floppy1(floppy1),
    _floppy2(floppy2)
{
}

void Tms5501Emulator::Start()
{
    Reset();

    _isRunning = true;
    _thread = std::thread(
        [this]() {
            while(_isRunning) {
                std::this_thread::sleep_for (std::chrono::milliseconds(20));

                for (uint8_t i = 0; i < 5; ++i) {
                    if (_context->count[i] > 0x00) {
                        _context->count[i] -= 1;
                        if (_context->count[i] == 0x00) {
                            _context->intStatus |= (i == 0) ? 0x01 :
                                         (i == 1) ? 0x02 :
                                         (i == 2) ? 0x08 :
                                         (i == 3) ? 0x40 :
                                                     0x80;
                        }
                    }
                }

                // interrupt the CPU if any timers expired
                CheckInterruptStatus();
            }
        }
    );
    _keyboardThread = std::thread(
        [this]() {
            std::this_thread::sleep_for (std::chrono::milliseconds(1000));

            while(_isRunning) {
                std::this_thread::sleep_for (std::chrono::milliseconds(17));

                _context->intStatus |= 0x04;
                CheckInterruptStatus();
            }
        }
    );
}

void Tms5501Emulator::Stop()
{
    _isRunning = false;
    _thread.join();
    _keyboardThread.join();
}

uint8_t Tms5501Emulator::Read(uint8_t port)
{
    uint8_t convertedPort = ConvertPort(port);

    switch (convertedPort)
    {
        case 0x0: {
            _context->sstatus &= ~0x08;    // clear bit 3: rx buffer full
            _context->intStatus &= ~0x10;  // clear serial rx buffer full
            CheckInterruptStatus();

            return _context->rxdata;
        }
        case 0x1: {
            //Read keyboard
            uint8_t keyboardRet = _keyboard->Read(_outport & 0xF);

            // the hardware unconditionally merges in the keyboard mode
            // keys if output bit 7 is set
            if (_outport & 0x80) {
                keyboardRet =
                    (0x0F & keyboardRet) |
                    (0xF0 & _keyboard->Read(16));
            }

            return keyboardRet;
        }
        case 0x2: {
            uint8_t retval = GetIntAddr();
            if (retval) {
                ClearIntAddr(retval);
            } else {
                retval = 0x00;
            }
            return retval;
        }
        case 0x3: {
            uint8_t retval = 0;

            if (IsSerialSelected()) {
                // this isn't modeled; just lie and say nothing is received
                // and we are ready to transmit
                retval = 0x14;
            } else {
                retval = _context->sstatus;
            }
            // bit 5 indicates if there is an unmasked interrupt pending
            retval = (retval & ~0x20) |
                     ((_context->intMask & _context->intStatus) ? 0x20 : 0x00);
            //if (0 && floppy_dbg) {
            //    console.log('T' + ccemu.getTickCount() +
            //                ':: 8080 read 5501 status(3): bit3=' + ((retval>>3) & 1) +
            //                ' @pc=' + cpu.pc.toString(16) +
            //                ' caller=' + (cpu.ram[cpu.sp] + 256*cpu.ram[cpu.sp+1]).toString(16)
            //               );
            //}
            // reading the status register clears the serial overrun flag
            _context->sstatus &= ~0x02;  // clear bit 1

            return retval;
        }
        default:
            break;
    }

    return 0x00;
}

void Tms5501Emulator::Write(uint8_t port, uint8_t data)
{
    uint8_t convertedPort = ConvertPort(port);
    switch (convertedPort)
    {
        case 0x4: { //TODO
            _context->dscCmd = data;
            if (data & 0x01) {
                // reset status
                _context->sstatus &= 0x35;  // clear bits 7,6,3,1
                // FIXME: serial transmit is set to high (marking)
                //        but it doesn't affect the tx buffer itself
                // the interrupt reg is cleared,
                // except tx buffer empty is set high
                //if (txCallbackTimer) {
                //    txCallbackTimer.cancel();
                //    txCallbackTimer = undefined;
                //}
                _context->txdataCount = 0;
                _context->intStatus = 0x20;  // tx buffer empty interrupt
            }
            // because the serial port isn't modeled, the break bit is
            // mostly ignored, but if we are modeling the soundware hardware,
            // which hangs on the TXD serial port bit, we need to let it know
            // this state might have changed.
            //if (serialSelected()) {
            //    audio.breakEvent((value >> 1) & 1);
            //}
            // bit 2: interrupt 7 select
            // I assume this is only programmed to 0 (use timer #5)
            // not data input bit 7 L->H transition as interrupt trigger.
            // Bit 7 corresponds to the caps lock state.
            //if (data & 0x04) {
            //    alert("Emulator modeling error: dscCmd & 0x04");
            //}
            // bit 3: interrupt acknowledge enable
            //    1=accept int ack
            //    0=ignore int ack
            //    FIXME: should this be modeled?
            //           in normal operation, the ccII will have this bit
            //           set, but what if some program messes with it?
            // bit 4: test control; normally low
            // bit 5: test control; normally low
            // bit 6: unused
            // bit 7: unused
        }
            break;
        case 0x5:
            _context->rate = data;
            break;

        case 0x6: { //TODO
            //if (0 && floppy_dbg) {
            //    console.log('T' + ccemu.getTickCount() +
            //                ':: 8080 sending serial data ' + value.toString(16) +
            //                ' @pc=' + cpu.pc.toString(16) +
            //                ' caller=' + (cpu.ram[cpu.sp] + 256*cpu.ram[cpu.sp+1]).toString(16)
            //               );
            //}

            // put it in the buffer
            _context->txdata = data;
            _context->txdataCount++;

            if (_context->txdataCount == 3) {
                // ignore the fact the tx buffer got clobbered
                //assert(!debugging, 'txdata buffer got clobbered');
                _context->txdataCount = 2;
            }
            if (_context->txdataCount == 2) {
                // there is something in the shift register and now the tx
                // buffer is fill too, so drop the tx buffer empty status
                _context->sstatus &= ~0x10;       // !xmit buffer empty
                _context->intStatus &= ~0x20;     // !serial character sent
            }
            if (_context->txdataCount == 1) {
                // send it immediately to the shift register
                //if (0 && floppy_dbg) {
                //    console.log('5501 wr() is sending txdata at T=' + ccemu.getTickCount());
                //}
                _context->txdata2 = _context->txdata;
                if (IsFloppy1Selected()) {
                    _floppy1->Write(0, _context->txdata2);
                } else if (IsFloppy2Selected()) {
                    _floppy2->Write(0, _context->txdata2);
                } else if (IsSerialSelected()) {
                    // make a callback to retire the tx byte
                //    var ticks = byteToTicks();
                //    txCallbackTimer = scheduler.oneShot(ticks, serialTxCallback,
                //                                        "txCB");
                //    // send it to the soundware emulation too just in case
                //    audio.txData(txdata2);
                }
                // it should already be set, but just in case ...
                _context->sstatus |= 0x10;       // xmit buffer empty
                // TBD: is the interrupt generated after the byte is sent, or
                //      any time the TX buffer is empty?  section 1.2 of the
                //      TMS5501 spec says:
                //          "Interrupts are also generated when the receive
                //           buffer is loaded and when the transmitter buffer
                //           is empty."
                _context->intStatus |= 0x20;     // transmit buffer empty
            }

            CheckInterruptStatus();
        }
            //TODO
            break;
        case 0x7:
            SetOutport(data);
            break;

        case 0x8:
            _context->intMask = data;
            CheckInterruptStatus();
            break;

        case 0x9:
        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
            _context->count[convertedPort - 0x9] = data;
            // if value=0, set interrupt immediately
            if (data == 0) {
                _context->intStatus |= (port == 0x9) ? 0x01 :
                             (port == 0xA) ? 0x02 :
                             (port == 0xB) ? 0x08 :
                             (port == 0xC) ? 0x40 :
                                              0x80;
                CheckInterruptStatus();
            }
            break;

        default:
            break;
    }
}

void Tms5501Emulator::Reset()
{
    SetOutport(0x00);
}

uint8_t Tms5501Emulator::ConvertPort(uint8_t port)
{
    return port & 0x0F;
}

void Tms5501Emulator::SetOutport(uint8_t value)
{
    _outport = value;
}

uint8_t Tms5501Emulator::GetIntAddr() {
    uint8_t masked = _context->intStatus & _context->intMask;

    if (IsBitSet(masked, 1))
        return 0xC7;
    if (IsBitSet(masked, 2))
        return 0xCF;
    if (IsBitSet(masked, 3))
        return 0xD7;
    if (IsBitSet(masked, 4))
        return 0xDF;
    if (IsBitSet(masked, 5))
        return 0xE7;
    if (IsBitSet(masked, 6))
        return 0xEF;
    if (IsBitSet(masked, 7))
        return 0xF7;
    if (IsBitSet(masked, 8))
        return 0xFF;

    return 0x0;
}

// clear an interrupt bit on interrupt ack, or when polled
void Tms5501Emulator::ClearIntAddr(uint8_t op) {
    uint8_t mask = 0xFF;
    if (op == 0xC7) mask = (uint8_t)~0x01;
    if (op == 0xCF) mask = (uint8_t)~0x02;
    if (op == 0xD7) mask = (uint8_t)~0x04;
    if (op == 0xDF) mask = (uint8_t)~0x08;
    if (op == 0xE7) mask = (uint8_t)~0x10;
    if (op == 0xEF) mask = (uint8_t)~0x20;
    if (op == 0xF7) mask = (uint8_t)~0x40;
    if (op == 0xFF) mask = (uint8_t)~0x80;

    _context->intStatus = _context->intStatus & mask;

    // when the first byte is sent to the serial port, the tx buffer
    // is still considered empty as that first byte immediately transfers
    // to the serial shift register.  So even if it gets acked, we should
    // leave it set.
    if (_context->txdataCount < 1) {
        _context->intStatus = _context->intStatus | 0x20;
    }

    // clearing a bit might affect int requeset
    CheckInterruptStatus();
}

void Tms5501Emulator::CheckInterruptStatus()
{
    if((_context->intStatus & _context->intMask) != 0)
    {
        uint8_t op = GetIntAddr();

        if (op > 0)
        {
            _context->intStatus = 0;

            _intel8080->RegisterInterrupt(op);

            ClearIntAddr(op);
        }
    }
}

bool Tms5501Emulator::IsBitSet(uint8_t data, uint8_t pos)
{
    return (data & (1 << pos)) != 0;
}

bool Tms5501Emulator::IsSerialSelected()
{
    return ((_context->outport >> 4) & 3) == 0;
}

bool Tms5501Emulator::IsFloppy1Selected()
{
    return ((_context->outport >> 4) & 3) == 1;
}

bool Tms5501Emulator::IsFloppy2Selected()
{
    return ((_context->outport >> 4) & 3) == 2;
}