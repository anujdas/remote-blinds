# remote-blinds

A wifi-enabled multi-blind Somfy remote control system accessible over MQTT

Compile with `make` (`platformio` required) and write to an ESP8266. This
project uses an RFM69 module to speak the custom 433.42MHz protocol Somfy
blinds use, but the code is customizable to speak OOK to a standard
fixed-frequency transmitter if one is available. In use, a Slack bot was used
to communicate with this device, enabling blinds-over-Slack control.
