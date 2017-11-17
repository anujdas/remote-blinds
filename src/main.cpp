#include <ESP8266WiFi.h>
#include <SPI.h>
#include <RFM69OOK.h>
#include <PubSubClient.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "config.h"
#include "Somfy.h"
#include "TxFifo.h"
#include "MessageHandler.h"

#define TX_PIN D0

BlindsConfig blinds_config;
Somfy somfy(&blinds_config);

RFM69OOK radio(SS, TX_PIN, true);
TxFifo fifo(&radio);

MessageHandler handler(&somfy, &fifo);

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

void setup() {
  Serial.begin(115200);

  // Fetch remote settings from EEPROM
  blinds_config.loadConfig();

  // Connect to WiFi, with config portal if necessary
  WiFiManager wifiManager;  // wifiManager.resetSettings();
  wifiManager.autoConnect();
  WiFi.mode(WIFI_STA);

  // Set up 433MHz radio
  radio.initialize();
  radio.transmitBegin();
  radio.setFrequencyMHz(433.42);

  // Initiailze pubsub client
  handler.setupMqttClient(&mqtt_client);

  // Seed PRNG; hppefully time to connect is random enough
  randomSeed(micros());

  Serial.println(F("Ready!"));
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("wifi dropped!");
    delay(5000);
  }
  if (!mqtt_client.connected() || !mqtt_client.loop()) {
    handler.setupMqttClient(&mqtt_client);
  }
  if (Serial.available() > 0) {
    handler.handleSerial((char) Serial.read());
  }
}
