#ifndef BlindsConfig_h
#define BlindsConfig_h

#include <Arduino.h>
#include <EEPROM.h>

/* magic bytes to verify loaded config */
#define CONFIG_VERSION "v1"

/* if any of this changes, bump CONFIG_VERSION */
#define EEPROM_START 0
#define EEPROM_SIZE  sizeof(MultiRemoteConfig)
#define MAX_REMOTES  3

#define REMOTE_BASE_ADDR   0x123456 // 24 bits
#define ROLLING_CODE_START 0x0123   // 16 bits

struct RemoteConfig {
  uint32_t remote_addr;
  uint16_t rolling_code;
};

struct MultiRemoteConfig {
  char version[2];
  uint8_t max_remotes;
  RemoteConfig remotes[MAX_REMOTES];
};

class BlindsConfig {
  public:
    BlindsConfig();

    void loadConfig();
    void saveConfig();

    uint32_t getRemoteAddr(uint8_t remote_num);
    uint16_t getRollingCode(uint8_t remote_num);
    uint16_t incrementRollingCode(uint8_t remote_num);

  private:
    MultiRemoteConfig config;
    void printConfig();
};

#endif
