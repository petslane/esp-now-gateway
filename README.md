# ESP-NOW Gateway

![VueJS bundle](https://github.com/petslane/esp-now-gateway/workflows/VueJS%20bundle/badge.svg)

![logo]

ESP-NOW Gateway with MQTT using 2 WeMos D1 minis where one WeMos D1 mini acts
as ESP-NOW device for sending and receiving ESP-NOW messages and second WeMos D1 mini acts as WiFi device that relays
ESP-NOW messages to/from MQTT channels. Provided Web UI allows to send ESP-NOW messages for testing purpose, monitor all
incoming messages, configure WiFi connection and define all known ESP-NOW devices.

## Getting Started

- GIT clone or download source
- open `data/config.json` and change `wifi` and `mqtt` parameters
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
- [PlatformIO][PIO-install] version 4
- MQTT broker/server
- OLED Shield (Optional)

### Configurations

All configurations (WiFi, MQTT, NOW devices) are stored in `data/config.json` file.

Web UI is implemented with [me-no-dev/ESPAsyncWebServer][ESPAsyncWebServer]. WebUI is protected with hardcoded
username/password that can be changed with variables `AUTH_USERNAME` and `AUTH_PASSWORD` in `platformio.ini` file.
WebUI itself is built, compressed and stored in `data/www` folder. Source code for WebUI is in `web_ui` folder.

If OLED shield is used, then uncomment `ENABLE_OLED_SHIELD` variable in `platformio.ini` file.

Two WeMos D1 minis communicate with each-other via dedicated software serial pins implemented with
[plerup/espsoftwareserial][espsoftwareserial].

All defined ESP-NOW devices are stored in `data/config.json` file in `devices` property. Devices can be defined via
Web UI, but also directly in `data/config.json` file and upload to SPIFFS. `devices` property value is JSON object with
property names being MAC address of remote ESP-NOW device and value being name of the device. TODO: Optionally, only
messages from defined devices will be relayed to MQTT.

Default gateway NOW MAC address is `30:30:30:30:30:30` and can be changed from WebUI. All remote devices must send
messages to configured MAC address.

### MQTT topics

Received NOW messages are published under `<topic_base>/<MAC>/message` MQTT topic.
For sending NOW message, publish message under `<topic_base>/<MAC>/send` MQTT topic.

Optionally, it's possible to assign numeric id to message that can be used to get feedback if messaged delivery succeeded or not.
To send message with id, publish message under `<topic_base>/<MAC>/send/id/<id>` MQTT topic and to get feedback of delivery status,
subscribe to `<topic_base>/<MAC>/message/report/<id>` MQTT topic.

`<topic_base>` is by default `now-gw` and changeable from WebUI.
`<MAC>` is mac address of NOW device, upper case with colons.
`<id>` is numeric value (max 4294967295).

Examples:
- Receiving ESP-NOW message from device with mac `60:01:94:35:05:aa` will publish that received message into
MQTT topic `now-gw/60:01:94:35:05:AA/message`
- Sending ESP-NOW message to device with mac `60:01:94:35:05:aa` require publishing message to MQTT topic
`now-gw/60:01:94:35:05:AA/send`

[PIO-install]: https://platformio.org/install/cli
[ESPAsyncWebServer]: https://github.com/me-no-dev/ESPAsyncWebServer
[espsoftwareserial]: https://github.com/plerup/espsoftwareserial
[logo]: image.png

### TODO

- OLED shield buttons - As I have only version 1 (without buttons), then there is no button functionality. In the future
I would like to implement some-kind of auto-pairing functionality for ESP-NOW devices, then these buttons would become
very handy.
- Ignore messages from unknown now devices
- Configure MQTT from WebUI
