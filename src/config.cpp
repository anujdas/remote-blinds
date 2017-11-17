#include "config.h"

BlindsConfig::BlindsConfig() {
}

void BlindsConfig::loadConfig() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(EEPROM_START, config);
  EEPROM.end();

  // Generate new config if none exists, or if existing one is incompatible
  if (strncmp(config.version, CONFIG_VERSION, sizeof(config.version)) != 0) {
    Serial.println(F("Generating new remote config"));
    strncpy(config.version, CONFIG_VERSION, sizeof(config.version));
    config.max_remotes = MAX_REMOTES;
    for (uint8_t i = 0; i < MAX_REMOTES; i++) {
      config.remotes[i].remote_addr = REMOTE_BASE_ADDR + i;
      config.remotes[i].rolling_code = ROLLING_CODE_START;
    }
    saveConfig();
    Serial.println(F("New config saved"));
  }

  printConfig();
}

void BlindsConfig::saveConfig() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(EEPROM_START, config);
  EEPROM.end();
};

void BlindsConfig::printConfig() {
  Serial.printf("Config: %c%c\r\n", config.version[0], config.version[1]);
  for (uint8_t i = 0; i < config.max_remotes; i++) {
    Serial.printf("Remote %d: addr 0x%x, rolling code %d\r\n",
        i + 1,
        config.remotes[i].remote_addr,
        config.remotes[i].rolling_code);
  }
}

uint32_t BlindsConfig::getRemoteAddr(uint8_t remote_num) {
  return config.remotes[remote_num].remote_addr;
}

uint16_t BlindsConfig::getRollingCode(uint8_t remote_num) {
  return config.remotes[remote_num].rolling_code;
}

uint16_t BlindsConfig::incrementRollingCode(uint8_t remote_num) {
  config.remotes[remote_num].rolling_code += 1;
  saveConfig();
  return config.remotes[remote_num].rolling_code;
}
