#ifndef Somfy_h
#define Somfy_h

#include <Arduino.h>
#include <EEPROM.h>

/* EEPROM base address and offsets for saved values */
#define EEPROM_BASE 0
#define ROLLING_CODE_OFFSET 0

/* Default values, if not set in EEPROM */
#define NEW_ROLLING_CODE 123

/* Button codes */
#define BTN_STOP 0x1
#define BTN_UP   0x2
#define BTN_DOWN 0x4
#define BTN_PROG 0x8

/* Command timings */
#define BIT_DURATION   302 // microseconds
#define WAKEUP_WIDTH   32  // BIT_DURATIONs = ~9415us
#define SILENCE_WIDTH  296 // BIT_DURATIONs = ~89565us
#define HW_SYNC_WIDTH  8   // BIT_DURATIONs = 2416us
#define SW_SYNC_WIDTH  15  // BIT_DURATIONs = ~4550us
#define SYM_WIDTH      2   // BIT_DURATIONs = 604us
#define FRAME_GAP_WDTH 101 // BIT_DURATIONs = ~30415us

/**
 * wakeup: 32 + 296 = 328 bits = 41 bytes
 * first: 2 * (2 * 8) + 15 + 2 + 56 * (2 * 2) + 101 = 374 bits = ~47 bytes
 * repeat: 7 * (2 * 8) + 15 + 2 + 56 * (2 * 2) + 101 = 454 bits = ~57 bytes
 * With three repeats, that's 41 + 47 + 3 * 57 = 259 bytes; 384 seems safe
 */
#define BUFFER_BYTES 384
#define COMMAND_REPEAT 3 // initial + 3 = total 4 attempts per command

class Somfy {
  public:
    Somfy(uint32_t remote_id, byte tx_pin);
    uint16_t getRollingCode();
    byte* buildFrame(byte button);
    byte* getBitstream();
    void broadcast(byte repeat);
  private:
    uint32_t remote_id;
    byte tx_pin;
    uint16_t rolling_code;
    byte frame[7];
    byte cksum;
    byte bitstream[BUFFER_BYTES];

    uint16_t readRollingCode();
    void writeRollingCode();
    void sendCommand(bool first_frame);
    void printFrame();
};

#endif
