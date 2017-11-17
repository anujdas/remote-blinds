#ifndef TxFifo_h
#define TxFifo_h

#include <Arduino.h>
#include <RFM69OOK.h>

/**
 * wakeup: 2 vals (preamble)
 * first: (4 * 2) (hw sync) + 2 (sw sync) + (2 * 56) (data) + 1 (gap) = 123 vals
 * repeat: (4 * 7) (hw sync) + 2 (sw sync) + (2 * 56) (data) + 1 (gap) = 143 vals
 * total: 2 + 123 + 3 * 143 = 554 vals, use 578 for margin
 */
#define MAX_STREAM_LENGTH 578

/* Pack val and length together on one byte: high bit is val */
#define LENGTH_MASK 0b01111111

class TxFifo {
  public:
    TxFifo(RFM69OOK* radio);

    void setBitDuration(uint16_t bit_duration);
    void clear();
    bool ready();
    void shift(bool val, uint16_t length);
    void transmit();

  private:
    uint8_t bitstream[MAX_STREAM_LENGTH];
    uint16_t stream_length;
    uint16_t bit_duration;
    RFM69OOK* radio;

    unsigned long timer;
    void delayUs(uint16_t us);
};

#endif
