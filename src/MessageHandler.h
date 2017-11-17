#ifndef MessageHandler_h
#define MessageHandler_h

#include <Arduino.h>
#include <PubSubClient.h>
#include "Somfy.h"
#include "TxFifo.h"

#define MQTT_QOS 1 // at least once

class MessageHandler {
  public:
    MessageHandler(Somfy* somfy, TxFifo* fifo);

    void setupMqttClient(PubSubClient* mqtt_client);
    void handleMqtt(char* topic, byte* payload, size_t length);
    void handleSerial(char input);

  private:
    uint8_t current_blind;
    Somfy* somfy;
    TxFifo* fifo;

    void handleCommand(uint8_t remote_num, char command);
};

#endif
