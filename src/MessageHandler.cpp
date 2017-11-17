#include "MessageHandler.h"

const char* mqtt_server = "anuj.corp.brigade.zone";
const uint16_t mqtt_port = 1883;
const char* mqtt_client_id = "BrigadeBlinds";

const char* connection_topic = "blinds/connected";
const char* command_topic = "blinds/+/command";
const uint8_t command_num_offset = 7;

MessageHandler::MessageHandler(Somfy* somfy, TxFifo* fifo) {
  this->current_blind = 0;
  this->somfy = somfy;
  this->fifo = fifo;
}

void MessageHandler::setupMqttClient(PubSubClient* mqtt_client) {
  mqtt_client->setServer(mqtt_server, mqtt_port);
  mqtt_client->setCallback([=](char* topic, byte* payload, size_t length) {
    this->handleMqtt(topic, payload, length);
  });

  Serial.print(F("Attempting MQTT connection... "));
  if (mqtt_client->connect(mqtt_client_id)) {
    mqtt_client->publish(connection_topic, mqtt_client_id);
    Serial.println(F("connected"));
    mqtt_client->subscribe(command_topic, MQTT_QOS);
    Serial.println(F("subscribed"));
  } else {
    Serial.printf("failed, rc=%d\r\n", mqtt_client->state());
  }
}

void MessageHandler::handleMqtt(char* topic, byte* payload, size_t length) {
  Serial.printf("Message received [%s]: ", topic);
  for (size_t i = 0; i < length; i++) Serial.print((char) payload[i]);
  Serial.println();

  if (strncmp(topic, command_topic, command_num_offset) == 0) {
    uint8_t remote_num = topic[command_num_offset] - '0';
    handleCommand(remote_num, (char) payload[0]);
  }
}

void MessageHandler::handleSerial(char input) {
  if (input == '0') {
    current_blind = 0;
    Serial.println(F("Current blind: 0"));
  } else if (input == '1') {
    current_blind = 1;
    Serial.println(F("Current blind: 1"));
  } else if (input == '2') {
    current_blind = 2;
    Serial.println(F("Current blind: 2"));
  } else if (input == 'u' || input == 'd' || input == 's' || input == 'p') {
    handleCommand(current_blind, input);
  } else {
    Serial.println(F("Help: <0-2> to select blind, <u/d/s> to control, <p> to program"));
  }
};

void MessageHandler::handleCommand(uint8_t remote_num, char command) {
  if (command == 'u') {
    somfy->buildFrame(BTN_UP, remote_num, fifo);
    Serial.println(F("Command: up"));
  } else if (command == 'd') {
    somfy->buildFrame(BTN_DOWN, remote_num, fifo);
    Serial.println(F("Command: down"));
  } else if (command == 's') {
    somfy->buildFrame(BTN_STOP, remote_num, fifo);
    Serial.println(F("Command: stop"));
  } else if (command == 'p') {
    somfy->buildFrame(BTN_PROG, remote_num, fifo);
    Serial.println(F("Command: program"));
  }

  if (fifo->ready()) {
    fifo->transmit();
    Serial.println(F("Transmitted!"));
  }
}
