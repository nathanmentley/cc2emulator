#include "Tms5501Emulator.h"

Tms5501Emulator::Tms5501Emulator(
    std::shared_ptr<IKeyboardEmulator> keyboard
):
    _keyboard(keyboard)
{
}

void Tms5501Emulator::Start()
{
    Reset();
}

void Tms5501Emulator::Stop()
{
}

uint8_t Tms5501Emulator::Read(uint8_t port)
{
    uint8_t convertedPort = ConvertPort(port);

    switch (convertedPort)
    {
        case 0x0:
            break;
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
        case 0x2:
            break;
        case 0x3:
            break;
        default:
            break;
    }
    /*
            switch(port)
            {
                case 0x0:
                    _sStatus = (byte)(_sStatus & ~0x08);
                    _intStatus = (byte)(_intStatus & ~0x10);
                    CheckInterruptStatus();
                    return _rxData;
                case 0x1:
                    byte retvalue = await _keyboard.Matrix((byte)(_outport & 0xF));

                    // data from keyboard connection J-1
                    // the hardware unconditionally merges in the keyboard mode
                    // keys if output bit 7 is set
                    if (IsBitSet(_outport, 7))
                    {
                        retvalue = (byte)(
                            (0x0F & retvalue) |
                            (0xF0 & await _keyboard.Matrix(16))
                        );
                    }

                    return retvalue;
                case 0x2:
                    byte retval = GetIntAddr();
                    if (retval > 0) {
                        ClearIntAddr(retval);
                    } else {
                        return 0;
                    }

                    return retval;
                case 0x3:
                    byte ret = 0x0;

                    if (IsSerialSelected())
                        ret = 0x14;
                    else
                        ret = _sStatus;

                    ret = (byte)((ret & ~0x20) | ((_intMask & _intStatus) > 0 ? 0x20 : 0x00));

                    _sStatus = (byte)(_sStatus & ~0x02);

                    return ret;
                default:
                    return 0;
            };
    `*/
    return convertedPort;
}

void Tms5501Emulator::Write(uint8_t port, uint8_t data)
{
    uint8_t convertedPort = ConvertPort(port);
    /*
            switch (convertedPort)
            {
                case 0x4:
                    _dscCmd = data;
                    if ((data & 0x01) > 0) {
                        // reset status
                        _sStatus = (byte)(_sStatus & 0x35);  // clear bits 7,6,3,1
                        // FIXME: serial transmit is set to high (marking)
                        //        but it doesn't affect the tx buffer itself
                        // the interrupt reg is cleared,
                        // except tx buffer empty is set high
                        //if (txCallbackTimer) {
                        //    txCallbackTimer.cancel();
                        //    txCallbackTimer = undefined;
                        //}
                        _txDataCount = 0;
                        _intStatus = 0x20;  // tx buffer empty interrupt
                    }
                    // because the serial port isn't modeled, the break bit is
                    // mostly ignored, but if we are modeling the soundware hardware,
                    // which hangs on the TXD serial port bit, we need to let it know
                    // this state might have changed.
                    if (IsSerialSelected()) {
                        //audio.breakEvent((value >> 1) & 1);
                    }
                    // bit 2: interrupt 7 select
                    // I assume this is only programmed to 0 (use timer #5)
                    // not data input bit 7 L->H transition as interrupt trigger.
                    // Bit 7 corresponds to the caps lock state.
                    if ((data & 0x04) > 0) {
                        _logger.LogCritical("Emulator modeling error: dscCmd & 0x04");
                    }
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
                    break;
                case 0x5:
                    _rate = data;
                    break;
                case 0x6:
                    _txData = data;
                    _txDataCount++;

                    if (_txDataCount == 3) {
                        // ignore the fact the tx buffer got clobbered
                        //assert(!debugging, 'txdata buffer got clobbered');
                        _txDataCount = 2;
                    }
                    if (_txDataCount == 2) {
                        // there is something in the shift register and now the tx
                        // buffer is fill too, so drop the tx buffer empty status
                        _sStatus = (byte)(_sStatus & ~0x10);       // !xmit buffer empty
                        _intStatus = (byte)(_intStatus & ~0x20);     // !serial character sent
                    }
                    if (_txDataCount == 1) {
                        // send it immediately to the shift register
                        // _txData2 = _txData;
                        // if (floppy0Selected()) {
                        //     floppy[0].txData(txdata2);
                        // } else if (floppy1Selected()) {
                        //     floppy[1].txData(txdata2);
                        // } else if (serialSelected()) {
                        //     // make a callback to retire the tx byte
                        //     var ticks = byteToTicks();
                        //     txCallbackTimer = scheduler.oneShot(ticks, serialTxCallback,
                        //                                         "txCB");
                        //     // send it to the soundware emulation too just in case
                        //     audio.txData(_txdata2);
                        // }
                        // it should already be set, but just in case ...
                        _sStatus = (byte)(_sStatus | 0x10);       // xmit buffer empty
                        // TBD: is the interrupt generated after the byte is sent, or
                        //      any time the TX buffer is empty?  section 1.2 of the
                        //      TMS5501 spec says:
                        //          "Interrupts are also generated when the receive
                        //           buffer is loaded and when the transmitter buffer
                        //           is empty."
                        _intStatus = (byte)(_intStatus | 0x20);     // transmit buffer empty
                    }
                    CheckInterruptStatus();
                    break;
                case 0x7:
                    SetOutport(data);
                    break;
                case 0x8:
                    _intMask = data;
                    CheckInterruptStatus();
                    break;
                case 0x9:
                case 0xA:
                case 0xB:
                case 0xC:
                case 0xD:
                    _count[convertedPort - 0x9] = data;

                    if (data == 0) {
                        _intStatus = convertedPort switch {
                            0x9 => (byte)(_intStatus | 0x01),
                            0xA => (byte)(_intStatus | 0x02),
                            0xB => (byte)(_intStatus | 0x08),
                            0xC => (byte)(_intStatus | 0x40),
                            0xD => (byte)(_intStatus | 0x80),
                            _ => _intStatus
                        };

                        CheckInterruptStatus();
                    }
                    break;
                default:
                    break;
            }
            */
    switch (convertedPort)
    {
        case 0x4:
        case 0x5:
        case 0x6:
            break;
        case 0x7:
            SetOutport(data);
            break;
        case 0x8:
        case 0x9:
        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
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