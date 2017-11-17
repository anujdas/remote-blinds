#include <ESP8266WiFi.h>
#include <SPI.h>
#include <RFM69OOK.h>
#include <Somfy.h>

#define REMOTE_ID 0x102030
#define TX_PIN D0

char ssid[] = "";  //  your network SSID (name)
char pass[] = "";  // your network password

RFM69OOK radio(SS, TX_PIN, true);
Somfy somfy(REMOTE_ID, TX_PIN);

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
  setupRadio();
  /* connectWifi(); */
}

void printFrame(byte* frame) {
  Serial.print("Frame: ");
  for (byte i = 0; i < 7; i++) {
    // Display leading zero when high nibble is 0
    if (frame[i] >> 4 == 0) Serial.print("0");
    Serial.print(frame[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
}

void loop() {
  if (Serial.available() > 0) {
    char input = (char) Serial.read();
    Serial.println("");
    if (input == 'u') {
      Serial.println("Up"); // Somfy is a French company, after all.
      somfy.buildFrame(BTN_UP);
    } else if (input == 's') {
      Serial.println("Stop");
      somfy.buildFrame(BTN_STOP);
    } else if (input == 'd') {
      Serial.println("Down");
      somfy.buildFrame(BTN_DOWN);
    } else if (input == 'p') {
      Serial.println("Program");
      somfy.buildFrame(BTN_PROG);
    } else {
      return;
    }

    somfy.broadcast(3);
    Serial.println("Sent!");
  }
}
