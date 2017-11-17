#ifndef Somfy_h
#define Somfy_h

#include <Arduino.h>
#include "config.h"
#include "TxFifo.h"

/* Button codes */
#define BTN_STOP 0x1
#define BTN_UP   0x2
#define BTN_DOWN 0x4
#define BTN_PROG 0x8

/* Command timings */
#define BIT_DURATION    302 // microseconds
#define WAKEUP_WIDTH    32  // BIT_DURATIONs = ~9415us
#define SILENCE_WIDTH   296 // BIT_DURATIONs = ~89565us
#define HW_SYNC_WIDTH   8   // BIT_DURATIONs = 2416us
#define SW_SYNC_WIDTH   15  // BIT_DURATIONs = ~4550us
#define SYM_WIDTH       2   // BIT_DURATIONs = 604us
#define FRAME_GAP_WIDTH 101 // BIT_DURATIONs = ~30415us

/* initial + 3 = total 4 attempts per command */
#define COMMAND_REPEAT 3

class Somfy {
  public:
    Somfy(BlindsConfig* config);

    void buildFrame(byte button, uint8_t remote_num, TxFifo* fifo);
    void printFrame();

  private:
    BlindsConfig* config;

    byte frame[7];
    byte cksum;

    void constructBitstream(TxFifo* fifo);
};

#endif
