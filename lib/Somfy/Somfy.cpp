#include "Somfy.h"

Somfy::Somfy(uint32_t remote_id, byte tx_pin) {
  this->tx_pin = tx_pin;
  this->remote_id = remote_id;

  readRollingCode();
  if (rolling_code < NEW_ROLLING_CODE) {
    rolling_code = NEW_ROLLING_CODE;
    writeRollingCode();
  }
}

uint16_t Somfy::getRollingCode() {
  return rolling_code;
}

/* Based on https://pushstack.wordpress.com/somfy-rts-protocol/
 *
 * Frame structure (seven-byte array):
 *     0       1        2       3       4       5       6
 * |-------|--------|-------|-------|-------|-------|-------|
 * |  key  |ctrl|cks|  Rolling Code |   Address(A0|A1|A3)   |
 * |-------|--------|-------|-------|-------|-------|-------|
 */
byte* Somfy::buildFrame(byte button) {
  frame[0] = 0xA0;              // "Encryption" key, only high nibble (0xA) matters... wtf
  frame[1] = button << 4;       // Button code in high nibble; low nibble will be checksum
  frame[2] = rolling_code >> 8; // Rolling code (big endian)
  frame[3] = rolling_code;      // Rolling code
  frame[4] = remote_id >> 16;   // Remote address (technically little endian, but doesn't matter)
  frame[5] = remote_id >> 8;    // Remote address
  frame[6] = remote_id;         // Remote address

  // Calculate 4-bit checksum (cks) as XOR of all nibbles w/ cks = 0
  cksum = frame[1] & 0x0F; // aka, 0
  for (byte i = 0; i < 7; i++) {
    cksum = cksum ^ frame[i] ^ (frame[i] >> 4);
  }
  frame[1] |= cksum & 0x0F; // mask low nibble and set on ctrl|cks byte

  // Obfuscate: XOR between each byte and the previous obfuscated one
  for (byte i = 1; i < 7; i++) {
    frame[i] ^= frame[i-1];
  }

  printFrame();

  rolling_code += 1;  // increment rolling code
  writeRollingCode(); // save new rolling code

  return frame;
}

byte* Somfy::getBitstream() {
  return bitstream; // TODO
}

void Somfy::broadcast(byte repeat) {
  //Wake-up pulse & Silence
  digitalWrite(tx_pin, HIGH);
  delayMicroseconds(WAKEUP_WIDTH * BIT_DURATION);
  digitalWrite(tx_pin, LOW);
  delayMicroseconds(SILENCE_WIDTH * BIT_DURATION);

  sendCommand(true);
  for (byte i = 0; i < repeat; i++) {
    sendCommand(false);
  }
}

uint16_t Somfy::readRollingCode() {
  EEPROM.begin(8);
  EEPROM.get(EEPROM_BASE + ROLLING_CODE_OFFSET, rolling_code);
  EEPROM.end();
}

void Somfy::writeRollingCode() {
  EEPROM.begin(8);
  EEPROM.put(EEPROM_BASE + ROLLING_CODE_OFFSET, rolling_code);
  EEPROM.end();
}

/* Based on https://pushstack.wordpress.com/somfy-rts-protocol/
 * Data is Manchester encoded, 604us per half-data-bit
 *
 *                                              604 us
 *                                                /
 * | 2416us | 2416us | 2416us | 2416us | 4550 us |  | 67648 us |  30415 us  |
 *
 * +--------+        +--------+        +---...---+
 * +        +--------+        +--------+         +--+XXXX...XXX+-----...-----
 *
 * |              hw. sync.            |   soft.    |          | Inter-frame
 * |                                   |   sync.    |   data   |     gap
 *
 */
void Somfy::sendCommand(bool first_frame) {
  // Hardware sync: two sync for the first frame, seven for the following ones.
  byte sync_repeat = first_frame ? 2 : 7;
  for (byte i = 0; i < sync_repeat; i++) {
    digitalWrite(tx_pin, HIGH);
    delayMicroseconds(HW_SYNC_WIDTH * BIT_DURATION);
    digitalWrite(tx_pin, LOW);
    delayMicroseconds(HW_SYNC_WIDTH * BIT_DURATION);
  }

  // Software sync
  digitalWrite(tx_pin, HIGH);
  delayMicroseconds(SW_SYNC_WIDTH * BIT_DURATION);
  digitalWrite(tx_pin, LOW);
  delayMicroseconds(SYM_WIDTH * BIT_DURATION);

  // Data: bits are sent one by one, starting with the MSB.
  bool bit;
  for (byte i = 0; i < 56; i++) {
    bit = ((frame[i/8] >> (7 - (i % 8))) & 1) == 1;
    digitalWrite(tx_pin, !bit); // use Manchester encoding (edges not values)
    delayMicroseconds(SYM_WIDTH * BIT_DURATION);
    digitalWrite(tx_pin, bit);
    delayMicroseconds(SYM_WIDTH * BIT_DURATION);
  }

  // Inter-frame silence
  digitalWrite(tx_pin, LOW);
  delayMicroseconds(FRAME_GAP_WDTH * BIT_DURATION);
}

void Somfy::printFrame() {
  Serial.print("Rolling code: ");
  Serial.println(rolling_code);
  Serial.print("Frame: ");
  for (byte i = 0; i < 7; i++) {
    // Display leading zero when high nibble is 0
    if (frame[i] >> 4 == 0) Serial.print("0");
    Serial.print(frame[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
}
