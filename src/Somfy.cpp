#include "Somfy.h"

Somfy::Somfy(BlindsConfig* config) {
  this->config = config;
}

/* Based on https://pushstack.wordpress.com/somfy-rts-protocol/
 *
 * Frame structure (seven-byte array):
 *     0       1        2       3       4       5       6
 * |-------|--------|-------|-------|-------|-------|-------|
 * |  key  |ctrl|cks|  Rolling Code |   Address(A0|A1|A3)   |
 * |-------|--------|-------|-------|-------|-------|-------|
 */
void Somfy::buildFrame(byte button, uint8_t remote_num, TxFifo* fifo) {
  uint32_t remote_addr = config->getRemoteAddr(remote_num);
  uint16_t rolling_code = config->incrementRollingCode(remote_num);

  frame[0] = 0xA0;              // "Encryption" key, only high nibble (0xA) matters... wtf
  frame[1] = button << 4;       // Button code in high nibble; low nibble will be checksum
  frame[2] = rolling_code >> 8; // Rolling code (big endian)
  frame[3] = rolling_code;      // Rolling code
  frame[4] = remote_addr >> 16; // Remote address (technically little endian, but doesn't matter)
  frame[5] = remote_addr >> 8;  // Remote address
  frame[6] = remote_addr;       // Remote address

  // Calculate 4-bit checksum (cks) as XOR of all nibbles w/ cks = 0
  cksum = frame[1] & 0x0F; // aka, 0
  for (byte i = 0; i < 7; i++) {
    cksum = cksum ^ frame[i] ^ (frame[i] >> 4);
  }
  frame[1] |= cksum & 0x0F; // mask low nibble and set on ctrl|cks byte

  printFrame(); // print post-checksum, pre-obfuscation

  // Obfuscate: XOR between each byte and the previous obfuscated one
  for (byte i = 1; i < 7; i++) {
    frame[i] ^= frame[i-1];
  }

  constructBitstream(fifo);
}

/* Based on https://pushstack.wordpress.com/somfy-rts-protocol/
 * Data is Manchester encoded, 604us per half-data-bit
 *
 *                                              604 us
 *                                                /
 * | 2416us | 2416us | 2416us | 2416us | 4550 us |  | 67648 us |  30415 us  |
 *
 * +--------+        +--------+        +---...---+
 * +        +--------+        +--------+         +--+XXXX..XXXX+-----...-----
 *
 * |              hw. sync.            |   soft.    |          | Inter-frame
 * |                                   |   sync.    |   data   |     gap
 *
 */
void Somfy::constructBitstream(TxFifo* fifo) {
  // Set the unit time multiplier on the FIFO bits
  fifo->setBitDuration(BIT_DURATION);

  // Wake-up pulse & silence (preamble)
  fifo->shift(HIGH, WAKEUP_WIDTH);
  fifo->shift(LOW, SILENCE_WIDTH);

  // Repeat frame couple times to be sure
  for (byte frame_num = 0; frame_num <= COMMAND_REPEAT; frame_num++) {
    // Hardware sync: two sync for the first frame, seven for the following ones.
    byte sync_repeat = frame_num == 0 ? 2 : 7;
    for (byte i = 0; i < sync_repeat; i++) {
      fifo->shift(HIGH, HW_SYNC_WIDTH);
      fifo->shift(LOW, HW_SYNC_WIDTH);
    }

    // Software sync
    fifo->shift(HIGH, SW_SYNC_WIDTH);
    fifo->shift(LOW, SYM_WIDTH);

    // Data: bits are sent Manchester-encoded starting with the MSB.
    bool bit;
    for (byte i = 0; i < 56; i++) {
      bit = bitRead(frame[i / 8], 7 - (i % 8));
      fifo->shift(!bit, SYM_WIDTH);
      fifo->shift(bit, SYM_WIDTH);
    }

    // Inter-frame silence
    fifo->shift(LOW, FRAME_GAP_WIDTH);
  }
}

void Somfy::printFrame() {
  Serial.print("Frame: ");
  for (byte i = 0; i < 7; i++) {
    // Display leading zero when high nibble is 0
    if (frame[i] >> 4 == 0) Serial.print("0");
    Serial.print(frame[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
}
