# esp-now-gateway

ESP-NOW Gateway with MQTT ([Homie v3][homie]) using 2 WeMos D1 minis where one WeMos D1 mini acts
as ESP-NOW device for sending and receiving ESP-NOW messages and second WeMos D1 mini acts as WiFi device that relays
ESP-NOW messages to/from MQTT channels. Provided Web UI allows to send ESP-NOW messages for testing purpose, monitor all
incoming messages and define all known ESP-NOW devices which make them available as Homie v3 Node which in turn makes
these remote ESP-NOW devices auto-discoverable by
[OpenHAB and HomeAssistant (WIP)][OpenHAB-homie]

## Getting Started

- GIT clone or download source
- open `data/homie/config.json` and change `wifi` and `mqtt` parameters
- connect first WeMos D1 mini with USB and upload sketch with command: (change `/dev/ttyUSB0` to USB port actually used)
  ```bash
  # Upload sketch
  platformio run -e web -t upload --upload-port /dev/ttyUSB0
  # Upload SPIFFS
  platformio run -e web -t uploadfs --upload-port /dev/ttyUSB0
  ```
- connect second WeMos D1 mini with USB and upload sketch with command: (change `/dev/ttyUSB0` to USB port actually used)
  ```bash
  # Upload sketch
  platformio run -e now -t upload --upload-port /dev/ttyUSB0
  ```
- mount 2 WeMos D1 minis and visit <http://now-gw.local>

### Prerequisites

- 2 WeMos D1 minis
- [PlatformIO][PIO-install]
- MQTT broker/server

### Configurations

This project uses [homieiot/homie-esp8266][Homie-esp] to manage WiFi connection and handle MQTT topics according to
[Homie v3.0.1 convention][Homie-spec]. For Homie, there is configuration file `data/homie/config.json` where
[WiFi, MQTT parameters][Homie-esp-config] can be set. This config file must be changed!

Web UI is implemented with [me-no-dev/ESPAsyncWebServer][ESPAsyncWebServer]. WebUI is protected with hardcoded
username/password that can be changed with variables `AUTH_USERNAME` and `AUTH_PASSWORD` in `platformio.ini` file.
WebUI itself is built, compressed and stored in `data/www` folder. Source code for WebUI will be released in separate
repository.

Two WeMos D1 minis communicate with each-other via dedicated software serial pins implemented with
[plerup/espsoftwareserial][espsoftwareserial].

All defined ESP-NOW devices are stored in `data/devices.json` file. Devices can be defined via Web UI, but also directly
in `data/devices.json` file and upload to SPIFFS. `data/devices.json` file content is JSON object with property names
being MAC address of remote ESP-NOW device and value being object with single property `"name"` to name that device. All
defined devices will be exposed as [Homie Node][homie-node] in MQTT.

Gateway NOW MAC address is hardcoded to `30:30:30:30:30:30`. So all remote devices must send messages to that MAC
address. TODO: Make Gateway address configurable.

### MQTT topics

Gateway MQTT follows [Homie v3.0.1][Homie] convention, this means:

- Receiving ESP-NOW message from defined device with mac `60:01:94:35:05:aa` will publish that received message into
MQTT topic `homie/gateway/6001943505AA/message`
- Sending ESP-NOW message to defined device with mac `60:01:94:35:05:aa` require publishing message to MQTT topic
`homie/gateway/6001943505AA/message/set`
- Receiving ESP-NOW message from undefined device with mac `35:05:aa:60:01:94` will publish that received message into
MQTT topic `homie/gateway/message/3505AA600194`
- Sending ESP-NOW message to undefined device with mac `35:05:aa:60:01:94` require publishing message to MQTT topic
`homie/gateway/message/3505AA600194/set`

[Homie]: https://homieiot.github.io/
[OpenHAB-homie]: https://homieiot.github.io/implementations/#home-automation
[PIO-install]: https://platformio.org/install/cli
[Homie-esp]: https://github.com/homieiot/homie-esp8266/tree/develop-v3
[Homie-spec]: https://homieiot.github.io/specification/
[Homie-esp-config]: https://homieiot.github.io/homie-esp8266/docs/develop-v3/configuration/json-configuration-file/
[ESPAsyncWebServer]: https://github.com/me-no-dev/ESPAsyncWebServer
[espsoftwareserial]: https://github.com/plerup/espsoftwareserial
[homie-node]: https://github.com/homieiot/convention/blob/develop/convention.md#nodes
