#pragma once

#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>
#include <config.hpp>
#include <ESP8266WiFi.h>

class MQTT {
  private:
    AsyncMqttClient mqttClient;
    Ticker mqttReconnectTimer;
    WiFiEventHandler wifiConnectHandler;
    WiFiEventHandler wifiDisconnectHandler;

    struct OutgoingMqttMessage {
        char mac[6];
        String * message;
    };
    std::vector<OutgoingMqttMessage> outgoingMqttMessages;

    void onWifiConnect(const WiFiEventStationModeGotIP &event) {
        Serial.println("MQTT: Connected to Wi-Fi.");
        connectToMqtt();
    }

    void onWifiDisconnect(const WiFiEventStationModeDisconnected &event) {
        mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while
                                     // reconnecting to Wi-Fi
    }

    void connectToMqtt() { mqttClient.connect(); }

    void onMqttConnect(bool sessionPresent) {
        Serial.println("MQTT: connected");

        String baseTopic = config.mqtt.baseTopic;
        if (!baseTopic.endsWith("/")) {
            baseTopic += "/";
        }

        mqttClient.publish((baseTopic + "connected").c_str(), 0, false, "ok");
        mqttClient.subscribe((baseTopic + "+/send").c_str(), 2);
    }

    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
        if (WiFi.isConnected()) {
            mqttReconnectTimer.once(1, std::bind(&MQTT::connectToMqtt, this));
        }
    }

    void onMqttMessage(char *topic, char *payload,
                       AsyncMqttClientMessageProperties properties, size_t len,
                       size_t index, size_t total) {
        Serial.println("Publish received.");
        Serial.print("  topic: ");
        Serial.println(topic);
        Serial.print("  qos: ");
        Serial.println(properties.qos);
        Serial.print("  dup: ");
        Serial.println(properties.dup);
        Serial.print("  retain: ");
        Serial.println(properties.retain);
        Serial.print("  len: ");
        Serial.println(len);
        Serial.print("  index: ");
        Serial.println(index);
        Serial.print("  total: ");
        Serial.println(total);

        char msg[len + 1];
        memcpy(msg, payload, len);
        msg[len] = '\0';
        String message(msg);

        String topicTemplate = config.mqtt.baseTopic;
        if (!topicTemplate.endsWith("/")) {
            topicTemplate += "/";
        }
        topicTemplate += "%02x:%02x:%02x:%02x:%02x:%02x/send";

        int mac[6];
        int i = sscanf(topic, topicTemplate.c_str(), &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        if (i != 6) {
            Serial.println("[mqtt] Received topic did not contain valid MAC address");
            return;
        }
        char mac2[6];
        for (uint8 i = 0; i < 6; i++) {
            mac2[i] = (char) mac[i];
        }

        Buffer::add_send_now_msg(mac2, message.c_str(), message.length(), 0);
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

  public:
    MQTT() {
        wifiConnectHandler = WiFi.onStationModeGotIP(
            std::bind(&MQTT::onWifiConnect, this, std::placeholders::_1));
        wifiDisconnectHandler = WiFi.onStationModeDisconnected(
            std::bind(&MQTT::onWifiDisconnect, this, std::placeholders::_1));

        mqttClient.onConnect(
            std::bind(&MQTT::onMqttConnect, this, std::placeholders::_1));
        mqttClient.onDisconnect(
            std::bind(&MQTT::onMqttDisconnect, this, std::placeholders::_1));
        mqttClient.onMessage(std::bind(
            &MQTT::onMqttMessage, this, std::placeholders::_1,
            std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
            std::placeholders::_5, std::placeholders::_6));
    }

    void setup() {
        String host = config.mqtt.host;
        int port = config.mqtt.port;

        IPAddress ipaddress;
        if (ipaddress.fromString(host)) {
            mqttClient.setServer(ipaddress, port);
        } else {
            mqttClient.setServer(host.c_str(), port);
        }
    }

    void loop() {
        if (mqttClient.connected() && outgoingMqttMessages.size() > 0) {
            OutgoingMqttMessage outMsg = outgoingMqttMessages.at(0);
            outgoingMqttMessages.erase(outgoingMqttMessages.begin()); // TODO: remove only if confirmed by broker

            String topic = config.mqtt.baseTopic;
            if (!topic.endsWith("/")) {
                topic += "/";
            }
            String mac = utils::macCharArrayToString(outMsg.mac);
            mac.toUpperCase();
            topic += mac;
            topic += "/message";

            mqttClient.publish(topic.c_str(), 1, false, outMsg.message->c_str(), outMsg.message->length());

            Serial.printf("[mqtt] Publishing to topic %s message %s\n", topic.c_str(), outMsg.message->c_str());

            delete outMsg.message;
        }
    }

    void send(const char *mac, const char *message, size_t len) {
        OutgoingMqttMessage o;
        memcpy(o.mac, mac, 6);

        char msg[len + 1];
        memcpy(msg, message, len);
        msg[len] = '\0';
        o.message = new String(msg);

        outgoingMqttMessages.push_back(o);

        Serial.printf("[mqtt] Queued message from %02X:%02X:%02X:%02X:%02X:%02X: %s\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], msg);
    }

    bool isConnected() {
        return mqttClient.connected();
    }
};
