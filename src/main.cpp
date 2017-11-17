#include <ESP8266WiFi.h>
#include <SPI.h>
#include <RFM69OOK.h>

#include "config.h"
#include "Somfy.h"

#define TX_PIN D0

char ssid[] = "";  //  your network SSID (name)
char pass[] = "";  // your network password

BlindsConfig blinds_config;
Somfy somfy(TX_PIN, &blinds_config);
RFM69OOK radio(SS, TX_PIN, true);

void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupRadio() {
  pinMode(TX_PIN, OUTPUT);
  radio.initialize();
  radio.transmitBegin();
  radio.setFrequencyMHz(433.42);
}

void setup() {
  Serial.begin(115200);
  blinds_config.loadConfig();
  setupRadio();
  /* connectWifi(); */
}

void loop() {
  if (Serial.available() > 0) {
    char input = (char) Serial.read();
    Serial.println("");
    if (input == 'u') {
      Serial.println("Up");
      somfy.buildFrame(BTN_UP, 1);
    } else if (input == 'd') {
      Serial.println("Down");
      somfy.buildFrame(BTN_DOWN, 1);
    } else if (input == 's') {
      Serial.println("Stop");
      somfy.buildFrame(BTN_STOP, 1);
    } else if (input == 'p') {
      Serial.println("Program");
      somfy.buildFrame(BTN_PROG, 1);
    } else {
      return;
    }

    somfy.broadcast(3);
    Serial.println("Sent!");
  }
}
