#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <buffer.hpp>
#include <stats.hpp>
#include <utils.hpp>

#if DEV_MODE == 1
#include <mqtt.hpp>
#include <websocket.hpp>
#endif // DEV_MODE == 1
#if DEV_MODE == 2
#endif // DEV_MODE == 2

#define SERIAL_PIN_1 D5
#define SERIAL_PIN_2 D4

typedef std::function<bool(String, const char *, uint8, unsigned long)> onNowMessageSendCallback;
typedef std::vector<onNowMessageSendCallback> onNowMessageSendCallbackVector;

class Com {
  private:
    SoftwareSerial *swSer;
    Stats *stats;
    long lastStatsReportTime = 0;
    long lastMacComTime = 0;
    bool statsChanged;
#if DEV_MODE == 1
    MQTT *mqtt;
    WebSocket *ws;
#endif
    uint8 bufferPosition = 0;
    char buffer[512];
    onNowMessageSendCallbackVector nowMessageSendCallbackVector;

    void receivedSerialData(const char *data, uint8 len) {
        Serial.print("[com] Received data: ");
        Serial.println(data);
        StaticJsonDocument<200> doc;
        DeserializationError err = deserializeJson(doc, data, len);

        if (err != DeserializationError::Ok) {
            Serial.println("[com] parseObject() failed");
            return;
        }

        JsonObject object = doc.as<JsonObject>();

        if (!object.containsKey("type")) {
            Serial.println("[com] Data container no cmd key");
            return;
        }

        uint8 type = doc["type"].as<uint8>();
        if (false) {
#if DEV_MODE == 1
        } else if (type == utils::msgType::request_mac) {
            this->send(utils::msgType::set_mac, 0, "mac", utils::macCharArrayToString(config.now.mac));
        } else if (type == utils::msgType::confirm_mac) {
            String mac = doc["mac"].as<String>();
            char mac_c[7];
            utils::macStringToCharArray(mac, mac_c);
            if (memcmp(mac_c, config.now.mac, 6) != 0) {
                memcpy(config.now.mac, mac_c, 6);
                Config::saveConfig();
            }
            config.now.changed = false;
        } else if (type == utils::msgType::received_now_message) {
            String msg = doc["msg"].as<String>();
            String from = doc["from"].as<String>();
            char mac[7];
            if (utils::macStringToCharArray(from, mac)) {
                this->mqtt->send(mac, msg.c_str(), msg.length(), OutgoingMqttMessageType::message);

                this->ws->send("type",
                               (const char *)"now_received",
                               "id",
                               doc["id"].as<long>(),
                               "from",
                               from.c_str(),
                               "msg",
                               msg.c_str());
            }
        } else if (type == utils::msgType::now_message_delivered) {
            unsigned long id = doc["id"].as<unsigned long>();
            String to = doc["to"].as<String>();
            Serial.printf("[com] Message %lu delivered\n", id);
            this->swSer->println("");
            this->ws->send("type", "sent", "id", id, "to", to.c_str(), "msg", doc["msg"].as<String>().c_str());

            char mac[7];
            utils::macStringToCharArray(to, mac);
            char msg[20];
            int len = sprintf(msg, "%lu", id);
            this->mqtt->send(mac, msg, len, OutgoingMqttMessageType::report_success);
        } else if (type == utils::msgType::now_message_not_delivered) {
            String to = doc["to"].as<String>();
            unsigned long id = doc["id"].as<unsigned long>();
            Serial.printf("[com] Message %lu NOT delivered\n", id);
            this->ws->send("type", "not_sent", "id", id, "to", to.c_str(), "msg", doc["msg"].as<String>().c_str());

            char mac[7];
            utils::macStringToCharArray(to, mac);
            char msg[20];
            int len = sprintf(msg, "%lu", id);
            this->mqtt->send(mac, msg, len, OutgoingMqttMessageType::report_fail);
        } else if (type == utils::msgType::stats) {
            Serial.printf("[com] Stats update from NOW node\n");
            String data = doc["data"].as<String>();
            stats->unpackRemoteData(data);
#endif // DEV_MODE == 1
#if DEV_MODE == 2
        } else if (type == utils::msgType::set_mac) {
            Serial.println("[com] Received NOW MAC address");
            String mac = doc["mac"].as<String>();
            char mac_c[7];
            if (utils::macStringToCharArray(mac, mac_c)) {
                onNowMacChangeCallback(mac_c);
            }
        } else if (type == (uint8)utils::msgType::send_now_message) {
            Serial.println("[com] Sending NOW message");
            String msg = doc["msg"].as<String>();
            String to = doc["to"].as<String>();
            unsigned long id = doc["id"].as<unsigned long>();

            std::for_each(nowMessageSendCallbackVector.begin(),
                          nowMessageSendCallbackVector.end(),
                          [&](onNowMessageSendCallback cb) { cb(to, msg.c_str(), (uint8)msg.length(), id); });
#endif // DEV_MODE == 2
        }
    }

    void sendJson(JsonDocument &doc) {
        String buf;
        buf += '\n';
        serializeJson(doc, buf);
        buf += '\n';

        this->swSer->print(buf);

        Serial.print("[com] Sending: ");
        Serial.print(buf.substring(1));
    }

  public:
#if DEV_MODE == 1
    Com(Stats *stats, WebSocket *ws, MQTT *mqtt) {
        this->swSer = new SoftwareSerial(SERIAL_PIN_1, SERIAL_PIN_2, false);
        this->mqtt = mqtt;
        this->ws = ws;
        this->stats = stats;
        this->statsChanged = false;
    }
#endif
#if DEV_MODE == 2
    typedef std::function<void(const char *)> OnNowMacChangeCallback;
    OnNowMacChangeCallback onNowMacChangeCallback;

    Com(Stats *stats) {
        swSer = new SoftwareSerial(SERIAL_PIN_2, SERIAL_PIN_1, false);
        this->stats = stats;
        this->statsChanged = false;

        stats->addChangeCallback([this]() { this->statsChanged = true; });
    }
#endif

    template <typename T1> void send(utils::msgType type, unsigned long id, String name1, T1 value1) {
        String s;
        send(type, id, name1, value1, s, s);
    }

    template <typename T1, typename T2>
    void send(utils::msgType type, unsigned long id, String name1, T1 value1, String name2, T2 value2) {
        String s;
        send(type, id, name1, value1, name2, value2, s, s);
    }

    template <typename T1, typename T2, typename T3>
    void send(utils::msgType type, unsigned long id, String name1, T1 value1, String name2, T2 value2, String name3,
              T3 value3) {
        DynamicJsonDocument doc(JSON_OBJECT_SIZE(6) + 6 + name1.length() + value1.length() + name2.length() +
                                value2.length() + name3.length() + value3.length());
        JsonObject json = doc.to<JsonObject>();

        json["type"].set((int)type);
        if (id) {
            json["id"].set<unsigned long>(id);
        }
        if (name1.length() > 0) {
            json[name1].set(value1);
        }
        if (name2.length() > 0) {
            json[name2].set(value2);
        }
        if (name3.length() > 0) {
            json[name3].set(value3);
        }

        sendJson(doc);
    }

    void setup() {
        swSer->begin(9600);
        swSer->print("\n");
    }

    void loop() {
#if DEV_MODE == 1
        if (config.now.changed && millis() - lastMacComTime > 1000) {
            this->send(utils::msgType::set_mac, 0, "mac", utils::macCharArrayToString(config.now.mac));
            lastMacComTime = millis();
        }
#endif
        bool hasFullData = false;
        while (this->swSer->available() > 0) {
            char c = (char)this->swSer->read();
            if (c == '\n') {
                if (!bufferPosition) {
                    continue;
                }
                hasFullData = true;
                break;
            }
            buffer[bufferPosition] = c;
            bufferPosition = (uint8)(bufferPosition + 1) % 512;
        }
        if (hasFullData) {
            receivedSerialData(buffer, bufferPosition);
            bufferPosition = 0;
            memset(buffer, '\0', 512);
        }
        if (!Buffer::is_buffer_empty() && !this->swSer->available()) {
            Buffer::Index dataIndex = Buffer::get_index(0);
            if (dataIndex.type == utils::msgType::send_now_message) {
                StaticJsonDocument<JSON_OBJECT_SIZE(6)> doc;
                JsonObject root = doc.to<JsonObject>();
                String mac = utils::macCharArrayToString(dataIndex.attr.send_now_message.mac);
                root["type"] = (int)utils::msgType::send_now_message;
                String msg = Buffer::get_data(0);
                root["msg"] = msg.c_str();
                root["to"] = mac.c_str();
                unsigned long id = dataIndex.id;
                if (!id) {
                    id = millis();
                }
                root["id"] = id;

                sendJson(doc);

#if DEV_MODE == 1
                this->ws->send("type", "sending", "id", id, "to", mac.c_str(), "msg", msg.c_str());
#endif

                Buffer::remove(0);
            }
        }

#if DEV_MODE == 2
        if (statsChanged && millis() - lastStatsReportTime > 300) {
            lastStatsReportTime = millis();
            statsChanged = false;
            send(utils::msgType::stats, 0, "data", stats->packRemoteData());
        }
#endif
    }

    void addOnNowMessageSendListener(onNowMessageSendCallback cb) {
        nowMessageSendCallbackVector.push_back(cb);
    }
};
