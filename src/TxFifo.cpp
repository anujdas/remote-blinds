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

bool TxFifo::push(bool val, uint16_t length) {
  if (stream_length + (length >> 7) >= MAX_STREAM_LENGTH) return false;

  bitstream[stream_length++] = (val << 7) | (length & LENGTH_MASK);
  for (uint8_t i = 0; i < (length >> 7); i++) {
    bitstream[stream_length++] = (val << 7) | LENGTH_MASK;
  }

  return true;
}

void TxFifo::transmit() {
  bool val;
  uint8_t length;

  for (uint16_t pos = 0; pos < stream_length; pos ++) {
    val = (bitstream[pos] >> 7) == 1;
    length = bitstream[pos] & LENGTH_MASK;
    radio->send(val);
    delayUs(length * bit_duration);
  }

  clear();
}

void TxFifo::delayUs(uint16_t us) {
  delayMicroseconds(us);
}
