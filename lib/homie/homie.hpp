#ifndef HOMIE_INIT_H
#define HOMIE_INIT_H

#include <Arduino.h>
#include <Homie.h>
#include <ESP8266mDNS.h>

class GWHomie {
private:
    HomieNode * nowNode;

    struct OutgoingMqttMessage {
        char mac[6];
        String * message;
    };

    std::vector<OutgoingMqttMessage> outgoingMqttMessages;

    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
        String topicTemplate = String(Homie.getConfiguration().mqtt.baseTopic);
        if (!topicTemplate.endsWith("/")) {
            topicTemplate += "/";
        }
        topicTemplate += Homie.getConfiguration().deviceId;
        topicTemplate += "/message/%02x%02x%02x%02x%02x%02x/set";

        int mac[6];
        int i = sscanf(topic, topicTemplate.c_str(), &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        if (i != 6) {
            Serial.println("[homie] Received topic did not contain valid MAC address");
            return;
        }
        char mac2[6];
        for (uint8 i = 0; i < 6; i++) {
            mac2[i] = (char) mac[i];
        }

        Buffer::add_send_now_msg(mac2, payload, len, 0);
    }

    bool macStringToCharArray(const char * from, char * to) {
        int mac[6];
        int i = sscanf(from, "%02x%02x%02x%02x%02x%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        if (i != 6) {
            return false;
        }

        for (uint8 i = 0; i < 6; i++) {
            to[i] = (char) mac[i];
        }

        return true;
    }

    bool sendMessage(const char * macName, const String& value) {
        char mac[7];
        if (!macStringToCharArray(macName, mac)) {
            return false;
        }
        Buffer::add_send_now_msg(mac, value.c_str(), value.length(), 0);

        return true;
    }
    bool globalInputHandler(const HomieNode& node, const HomieRange& range, const String& property, const String& value) {
        Homie.getLogger() << "Received on node " << node.getId() << ": " << property << " = " << value << endl;

        if (property.equals("message")) {
            sendMessage(node.getId(), value);
        }

        return true;
    }

    void setupNodes() {
        nowNode->advertise("now_sent_success")
                .setName("NOW messages successfully sent")
                .setDatatype("integer")
                .setRetained(true);
        nowNode->advertise("now_sent_failed")
                .setName("NOW messages failed sent")
                .setDatatype("integer")
                .setRetained(true);
        nowNode->advertise("now_received")
                .setName("NOW messages received")
                .setDatatype("integer")
                .setRetained(true);

        if (!SPIFFS.exists("/devices.json")) {
            Serial.printf("[homie] devices.json file does not exist\n");
            return;
        }

        File f = SPIFFS.open("/devices.json", "r");
        if (!f) {
            Serial.printf("[homie] unable to open devices.json file\n");
            return;
        }

        StaticJsonBuffer<2048> jsonBuffer;
        JsonObject &fileRoot = jsonBuffer.parseObject(f);

        if (!fileRoot.success()) {
            Serial.printf("[homie] Invalid devices.json content\n");
            return;
        }

        for (auto kv : fileRoot) {
            char * name = (char *) fileRoot.get<JsonObject>(kv.key).get<const char*>("name");

            char mac[6];
            if (!utils::macStringToCharArray(kv.key, mac)) {
                continue;
            }

            char *devName = new char[strlen(name) + 1];
            memset(devName, '\0', strlen(name) + 1);
            memcpy(devName, name, strlen(name));
            char *devMac = new char[13];
            memset(devMac, '\0', 13);
            sprintf(devMac, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

            HomieNode *buttonNode = new HomieNode(devMac, "now device", "NOW device");
            buttonNode->advertise("message")
                    .setName(devName)
                    .setDatatype("string")
                    .setRetained(false)
                    .settable();
        }

    }

    void onHomieEvent(const HomieEvent& event) {
        if (event.type == HomieEventType::WIFI_CONNECTED) {
            MDNS.begin("now-gw");
            MDNS.addService("http","tcp",80);
        } else if (event.type == HomieEventType::WIFI_DISCONNECTED) {
            MDNS.end();
        } else if (event.type == HomieEventType::MQTT_READY) {
            Serial.println("[homie] MQTT ready");

            String topic = String(Homie.getConfiguration().mqtt.baseTopic);
            if (!topic.endsWith("/")) {
                topic += "/";
            }
            topic += Homie.getConfiguration().deviceId;
            topic += "/message/+/send";

            Homie.getMqttClient().subscribe(topic.c_str(), 0);

            Serial.printf("[homie] Subscribed to topic: %s\n", topic.c_str());
        } else if (event.type == HomieEventType::MQTT_DISCONNECTED) {
            Serial.println("[homie] MQTT disconnected");
        }
    }
public:
    GWHomie() {
        nowNode = new HomieNode("now", "ESP-NOW Gateway", "Gateway");
    }

    void setup() {
        Homie_setFirmware("esp-now-gateway", "1.0.0");
        Homie.setHomieBootMode(HomieBootMode::NORMAL);

        Homie.onEvent(std::bind(
                &GWHomie::onHomieEvent,
                this,
                std::placeholders::_1
        ));

        Homie.getMqttClient().onMessage(std::bind(
                &GWHomie::onMqttMessage,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4,
                std::placeholders::_5,
                std::placeholders::_6
        ));

        SPIFFS.begin();
        setupNodes();

        Homie.setGlobalInputHandler(std::bind(
                &GWHomie::globalInputHandler,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4
        ));

        Homie.setup();

        outgoingMqttMessages.reserve(50);
    }

    void loop() {
        Homie.loop();
        MDNS.update();

        if (Homie.getMqttClient().connected() && outgoingMqttMessages.size() > 0) {
            OutgoingMqttMessage outgoingMqttMessage = outgoingMqttMessages.at(0);
            outgoingMqttMessages.erase(outgoingMqttMessages.begin());

            String topic = String(Homie.getConfiguration().mqtt.baseTopic);
            if (!topic.endsWith("/")) {
                topic += "/";
            }
            topic += Homie.getConfiguration().deviceId;
            topic += "/message/";
            String mac = utils::macCharArrayToString(outgoingMqttMessage.mac);
            mac.toUpperCase();
            topic += mac;

            Homie.getMqttClient().publish(topic.c_str(), 1, false, outgoingMqttMessage.message->c_str(), outgoingMqttMessage.message->length());

            Serial.printf("[homie] Publishing to topic %s message %s\n", topic.c_str(), outgoingMqttMessage.message->c_str());

            delete outgoingMqttMessage.message;
        }
    }

    void send(const char* mac, const char* message, size_t len) {
        OutgoingMqttMessage o;
        memcpy(o.mac, mac, 6);

        char msg[len + 1];
        memcpy(msg, message, len);
        msg[len] = '\0';
        o.message = new String(msg);

        outgoingMqttMessages.push_back(o);

        Serial.printf("[homie] Queued message from %02X:%02X:%02X:%02X:%02X:%02X: %s\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], msg);
    }
};

#endif // HOMIE_INIT_H
