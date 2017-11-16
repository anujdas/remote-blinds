#ifndef Somfy_h
#define Somfy_h

#include <Arduino.h>
#include <EEPROM.h>

/* EEPROM base address and offsets for saved values */
#define EEPROM_BASE 0
#define ROLLING_CODE_OFFSET 0
#define REMOTE_ID_OFFSET ROLLING_CODE_OFFSET + sizeof(uint16_t)

/* Default values, if not set in EEPROM */
#define NEW_ROLLING_CODE 123

/* Button codes */
#define BTN_STOP 0x1
#define BTN_UP   0x2
#define BTN_DOWN 0x4
#define BTN_PROG 0x8

#define SYM_WIDTH 640 // microseconds

class Somfy {
  public:
    Somfy(uint32_t _remote_id);
    byte* buildFrame(byte button);
    void sendCommand(byte sync);
    void printRollingCode();
  private:
    uint32_t remote_id;
    uint16_t rolling_code;
    byte frame[7];
    byte cksum;

    uint16_t readRollingCode();
    uint16_t writeRollingCode();
};

#endif
