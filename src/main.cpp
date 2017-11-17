#include <ESP8266WiFi.h>
#include <SPI.h>
#include <RFM69OOK.h>
#include <Wire.h>
#include <SSD1306.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "config.h"
#include "Somfy.h"
#include "TxFifo.h"
#include "Ui.h"

#define I2C_ADDRESS 0x3C
#define SDA D2
#define SCL D1

#define TX_PIN D0

BlindsConfig blinds_config;
Somfy somfy(&blinds_config);
RFM69OOK radio(SS, TX_PIN, true);
TxFifo fifo(&radio);

SSD1306 display(I2C_ADDRESS, SDA, SCL);
Ui ui(&display);

uint8_t current_blind = 0;

void setup() {
  Serial.begin(115200);

  // Init display
  ui.init();

  // Fetch remote settings from EEPROM
  blinds_config.loadConfig();

  // Connect to WiFi, with config portal if necessary
  WiFiManager wifiManager;
  wifiManager.autoConnect();

  // Set up 433MHz radio
  radio.initialize();
  radio.transmitBegin();
  radio.setFrequencyMHz(433.42);

  Serial.println(F("Ready!"));
}

void loop() {
  if (Serial.available() > 0) {
    char input = (char) Serial.read();

    if (input == '0') {
      current_blind = 0;
      Serial.println(F("Current blind: 0"));
    } else if (input == '1') {
      current_blind = 1;
      Serial.println(F("Current blind: 1"));
    } else if (input == '2') {
      current_blind = 2;
      Serial.println(F("Current blind: 2"));
    } else if (input == 'u') {
      somfy.buildFrame(BTN_UP, current_blind, &fifo);
      fifo.transmit();
      Serial.println(F("Transmitted: up"));
    } else if (input == 'd') {
      somfy.buildFrame(BTN_DOWN, current_blind, &fifo);
      fifo.transmit();
      Serial.println(F("Transmitted: down"));
    } else if (input == 's') {
      somfy.buildFrame(BTN_STOP, current_blind, &fifo);
      fifo.transmit();
      Serial.println(F("Transmitted: stop"));
    } else if (input == 'p') {
      somfy.buildFrame(BTN_PROG, current_blind, &fifo);
      fifo.transmit();
      Serial.println(F("Transmitted: program"));
    } else {
      Serial.println(F("Help: <0-2> to select blind, <u/d/s> to control, <p> to program"));
      return;
    }
  }
}
