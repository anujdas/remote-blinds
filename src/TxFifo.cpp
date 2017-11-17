#include "TxFifo.h"

TxFifo::TxFifo(RFM69OOK* radio) {
  this->radio = radio;
  setBitDuration(0); // a silly default
  clear();
}

void TxFifo::setBitDuration(uint16_t bit_duration) {
  this->bit_duration = bit_duration;
}

void TxFifo::clear() {
  stream_length = 0;
}

bool TxFifo::ready() {
  return stream_length != 0;
}

void TxFifo::shift(bool val, uint16_t length) {
  if (stream_length + (length >> 7) >= MAX_STREAM_LENGTH) {
    Serial.println(F("FIFO overflow, dropping vals"));
    return;
  }

  bitstream[stream_length++] = (val << 7) | (length & LENGTH_MASK);
  for (uint16_t i = 0; i < (length >> 7); i++) {
    bitstream[stream_length++] = (val << 7) | LENGTH_MASK;
  }
}

void TxFifo::transmit() {
  bool val;
  uint8_t length;

  Serial.printf("Transmitting %d dataframes\r\n", stream_length);
  for (uint16_t pos = 0; pos < stream_length; pos++) {
    val = (bitstream[pos] >> 7) == 1;
    length = bitstream[pos] & LENGTH_MASK;
    radio->send(val);
    delayUs(length * bit_duration);
  }

  clear();
}

/** delayMicroseconds() blocks all execution, which can cause Wifi issues and
 * WDT restarts; try to use delay() for parts of larger waits to prevent this
 * from happening.
 */
void TxFifo::delayUs(uint16_t us) {
  timer = micros();
  if (us > 2000) {
    while (micros() - timer < us - 500) yield();
  }
  delayMicroseconds(us - (micros() - timer));
}
