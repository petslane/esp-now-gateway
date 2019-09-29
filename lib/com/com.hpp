#ifndef COM_INIT_H
#define COM_INIT_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "buffer.hpp"
#include "utils.hpp"

#if DEV_MODE == 1
#include <homie.hpp>
#include "websocket.hpp"
#endif // DEV_MODE == 1
#if DEV_MODE == 2
#endif // DEV_MODE == 2

#define SERIAL_PIN_1 D1
#define SERIAL_PIN_2 D2

typedef std::function<bool(String, const char *, uint8, unsigned long)> onNowMessageSendCallback;
typedef std::vector<onNowMessageSendCallback> onNowMessageSendCallbackVector;

class Com {
private:
    SoftwareSerial * swSer;
#if DEV_MODE == 1
    GWHomie * homie;
#endif
    uint8 bufferPosition = 0;
    char buffer[512];
    onNowMessageSendCallbackVector nowMessageSendCallbackVector;

    void receivedSerialData(const char *data, uint8 len) {
        Serial.print("[com] Received data: ");
        Serial.println(data);
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& doc = jsonBuffer.parseObject(data);

        if (!doc.success()) {
            Serial.println("[com] parseObject() failed");
            return;
        }

        if (!doc.containsKey("type")) {
            Serial.println("[com] Data container no cmd key");
            return;
        }

        uint8 type = doc["type"].as<uint8>();
        if (false) {
#if DEV_MODE == 1
        } else if (type == utils::msgType::received_now_message) {
            String msg = doc["msg"].as<String>();
            String from = doc["from"].as<String>();
            char mac[7];
            if (utils::macStringToCharArray(from, mac)) {
                this->homie->send(mac, msg.c_str(), msg.length());
                ws::ws.textAll("Message received");
            }
        } else if (type == utils::msgType::now_message_delivered) {
            unsigned long id =  doc.get<unsigned long>("id");
            Serial.printf("[com] Message %i delivered\n", id);
            this->swSer->println("");
            ws::ws.textAll("Message delivered");
        } else if (type == utils::msgType::now_message_not_delivered) {
            unsigned long id =  doc.get<unsigned long>("id");
            Serial.printf("[com] Message %i NOT delivered\n", id);
            ws::ws.textAll("Message not delivered");
#endif // DEV_MODE == 1
#if DEV_MODE == 2
        } else if (type == (uint8) utils::msgType::send_now_message) {
            Serial.println("[com] Sending NOW message");
            String msg = doc["msg"].as<String>();
            String to = doc["to"].as<String>();
            unsigned long id = doc["id"].as<unsigned long>();

            std::for_each(nowMessageSendCallbackVector.begin(), nowMessageSendCallbackVector.end(), [&](onNowMessageSendCallback cb) {
                cb(to, msg.c_str(), (uint8) msg.length(), id);
            });
#endif // DEV_MODE == 2
        }
    }

public:
#if DEV_MODE == 1
    Com(GWHomie * h) {
        this->swSer = new SoftwareSerial(SERIAL_PIN_1, SERIAL_PIN_2, false, 256);
        this->homie = h;
    }
#endif
#if DEV_MODE == 2
    Com() {
        swSer = new SoftwareSerial(SERIAL_PIN_2, SERIAL_PIN_1, false, 256);
    }
#endif
    void send(utils::msgType type, unsigned long id, String name1, String value1) {
        String s;
        send(type, id, name1, value1, s, s);
    }
    void send(utils::msgType type, unsigned long id, String name1, String value1, String name2, String value2) {
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& json = jsonBuffer.createObject();

        json.set("type", (int) type);
        if (id) {
            json.set("id", id);
        }
        if (name1.length() > 0) {
            json.set(name1, value1);
        }
        if (name2.length() > 0) {
            json.set(name2, value2);
        }

        this->swSer->print("\n");
        json.printTo(*this->swSer);
        this->swSer->print("\n");

        Serial.print("[com] Sending: ");
        json.printTo(Serial);
        Serial.println();
    }

    void setup() {
        swSer->begin(9600);
        swSer->print("\n");
    }

    void loop() {
        bool hasFullData = false;
        while (this->swSer->available() > 0) {
            char c = (char) this->swSer->read();
            if (c == '\n') {
                if (!bufferPosition) {
                    continue;
                }
                hasFullData = true;
                break;
            }
            buffer[bufferPosition] = c;
            bufferPosition++;
        }
        if (hasFullData) {
            receivedSerialData(buffer, bufferPosition);
            bufferPosition = 0;
            memset(buffer, '\0', 512);
        }
        if (!Buffer::is_buffer_empty() && !this->swSer->available()) {
            Buffer::Index dataIndex = Buffer::get_index(0);
            if (dataIndex.type == utils::msgType::send_now_message) {
                StaticJsonBuffer<300> jsonBuffer;
                JsonObject& root = jsonBuffer.createObject();
                root["type"] = (int) utils::msgType::send_now_message;
                const String string = Buffer::get_data(0);
                root["msg"] = string;
                root["to"] = utils::macCharArrayToString(dataIndex.attr.send_now_message.mac);
                if (dataIndex.id) {
                    root["id"] = dataIndex.id;
                }
                this->swSer->println();
                root.printTo(*this->swSer);
                this->swSer->println();

                Serial.print("[com] Sending: ");
                root.printTo(Serial);
                Serial.println();

                Buffer::remove(0);
            }

        }
    }

    void addOnNowMessageSendListener(onNowMessageSendCallback cb) {
        nowMessageSendCallbackVector.push_back(cb);
    }
};

#endif // COM_INIT_H
