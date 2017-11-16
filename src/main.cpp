#include <ESP8266WiFi.h>
#include <Somfy.h>

#define REMOTE_ID 0x102030

char ssid[] = "";  //  your network SSID (name)
char pass[] = "";       // your network password

Somfy somfy(REMOTE_ID);

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
};

void setup() {
  Serial.begin(115200);

  connectWifi();
}

void printFrame(byte* frame) {
  Serial.print("Frame         : ");
  for (byte i = 0; i < 7; i++) {
    if (frame[i] >> 4 == 0) { // Displays leading zero in case the most significant
      Serial.print("0");      // nibble is a 0.
    }
    Serial.print(frame[i], HEX); Serial.print(" ");
  }
}

void loop() {
  somfy.printRollingCode();
  printFrame(somfy.buildFrame(BTN_STOP));

  delay(1000);
}
