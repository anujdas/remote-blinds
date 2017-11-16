#include "Somfy.h"

Somfy::Somfy(uint32_t _remote_id) {
  remote_id = _remote_id;

  if (readRollingCode() < NEW_ROLLING_CODE) {
    rolling_code = NEW_ROLLING_CODE;
    writeRollingCode();
  }
}

uint16_t Somfy::readRollingCode() {
  return EEPROM.get(EEPROM_BASE + ROLLING_CODE_OFFSET, rolling_code);
}

uint16_t Somfy::writeRollingCode() {
  return EEPROM.put(EEPROM_BASE + ROLLING_CODE_OFFSET, rolling_code);
}

void Somfy::printRollingCode() {
  Serial.println("Rolling code: ");
  Serial.println(readRollingCode());
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

  rolling_code += 1;  // increment rolling code
  writeRollingCode(); // save new rolling code

  return frame;
}

/* void Somfy::sendCommand(byte sync) { */
/*   if (sync == 2) { // Only with the first frame. */
/*     //Wake-up pulse & Silence */
/*     PORTD |= 1 << PORT_TX; */
/*     delayMicroseconds(9415); */
/*     PORTD &= !(1 << PORT_TX); */
/*     delayMicroseconds(89565); */
/*   } */

/*   // Hardware sync: two sync for the first frame, seven for the following ones. */
/*   for (int i = 0; i < sync; i++) { */
/*     PORTD |= 1 << PORT_TX; */
/*     delayMicroseconds(4 * SYMBOL); */
/*     PORTD &= !(1 << PORT_TX); */
/*     delayMicroseconds(4 * SYMBOL); */
/*   } */

/*   // Software sync */
/*   PORTD |= 1 << PORT_TX; */
/*   delayMicroseconds(4550); */
/*   PORTD &= !(1 << PORT_TX); */
/*   delayMicroseconds(SYMBOL); */

/*   // Data: bits are sent one by one, starting with the MSB. */
/*   for (byte i = 0; i < 56; i++) { */
/*     if (((frame[i/8] >> (7 - (i % 8))) & 1) == 1) { */
/*       PORTD &= !(1 << PORT_TX); */
/*       delayMicroseconds(SYMBOL); */
/*       PORTD ^= 1 << 5; */
/*       delayMicroseconds(SYMBOL); */
/*     } */
/*     else { */
/*       PORTD |= (1 << PORT_TX); */
/*       delayMicroseconds(SYMBOL); */
/*       PORTD ^= 1 << 5; */
/*       delayMicroseconds(SYMBOL); */
/*     } */
/*   } */

/*   PORTD &= !(1 << PORT_TX); */
/*   delayMicroseconds(30415); // Inter-frame silence */
/* } */
