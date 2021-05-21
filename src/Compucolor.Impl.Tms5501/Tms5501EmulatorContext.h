#pragma once

#include <memory>
#include <vector>

namespace Compucolor::Impl::Tms5501 {
    class Tms5501EmulatorContext
    {
        public:
            uint8_t rxdata = 0x00;      // serial rx data
            uint8_t txdata = 0x00;      // serial tx data, buffered
            uint8_t txdata2 = 0x00;     // serial tx data, in bit shifter
            uint8_t txdataCount = 0;    // double buffer depth
            uint8_t outport = 0x00;     // parallel output port
            uint8_t sstatus = 0x00;     // chip status:
                              //   bit 0: frame error
                              //   bit 1: overrun error
                              //   bit 2: serial rcvd
                              //   bit 3: rcv buffer loaded
                              //   bit 4: xmit buffer empty
                              //   bit 5: interrupt pending
                              //   bit 6: full bit detect
                              //   bit 7: start bit detect

            uint8_t rate = 0x00;        // serial baud rate control
                              //   bit 0:  110 baud
                              //   bit 1:  150 baud
                              //   bit 2:  300 baud
                              //   bit 3: 1200 baud
                              //   bit 4: 2400 baud
                              //   bit 5: 4800 baud
                              //   bit 6: 9600 baud
                              //   bit 7: 0=one stop bit, 1=two stop bits

            uint8_t intMask = 0x00;     // interrupt enable mask
                              //   bit 0: 1= enable timer #1 interrupt
                              //   bit 1: 1= enable timer #2 interrupt
                              //   bit 2: 1= enable interrupt external sensor
                              //             fires at 60 Hz (50 in europe)
                              //   bit 3: 1= enable timer #3 interrupt
                              //   bit 4: 1= enable serial rx interrupt
                              //   bit 5: 1= enable serial tx interrupt
                              //   bit 6: 1= enable timer #4 interrupt
                              //   bit 7: 1= enable timer #5 interrupt
                              //             (or parallel input,
                              //              depending on other mode bit)
            uint8_t intStatus = 0x00;   // interrupt pending flags (like intMask)
                              // not a CPU accessible register
            uint8_t dscCmd = 0x00;      // discrete command

            std::vector<uint8_t> count;// = [0x00, 0x00, 0x00, 0x00, 0x00], // current counter

            Tms5501EmulatorContext();
    };
}