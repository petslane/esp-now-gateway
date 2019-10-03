#ifndef COM_INIT_H
#define COM_INIT_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "buffer.hpp"
#include "utils.hpp"
#include "stats.hpp"

#if DEV_MODE == 1
#include <homie.hpp>
#include "websocket.hpp"
#endif // DEV_MODE == 1
#if DEV_MODE == 2
#endif // DEV_MODE == 2

#define SERIAL_PIN_1 D5
#define SERIAL_PIN_2 D4

typedef std::function<bool(String, const char *, uint8, unsigned long)> onNowMessageSendCallback;
typedef std::vector<onNowMessageSendCallback> onNowMessageSendCallbackVector;

class Com {
private:
    SoftwareSerial * swSer;
    Stats * stats;
    bool statsChanged;
#if DEV_MODE == 1
    GWHomie * homie;
    WebSocket * ws;
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
                ws->textAll("Message received");
            }
        } else if (type == utils::msgType::now_message_delivered) {
            unsigned long id =  doc.get<unsigned long>("id");
            Serial.printf("[com] Message %i delivered\n", id);
            this->swSer->println("");
            ws->textAll("Message delivered");
        } else if (type == utils::msgType::now_message_not_delivered) {
            unsigned long id =  doc.get<unsigned long>("id");
            Serial.printf("[com] Message %i NOT delivered\n", id);
            ws->textAll("Message not delivered");
        } else if (type == utils::msgType::stats) {
            Serial.printf("[com] Stats update from NOW node\n");
            stats->setNowSentMessagesSuccessful(doc.get<int>("success"));
            stats->setNowSentMessagesFailed(doc.get<int>("failed"));
            stats->setNowSentMessagesReceived(doc.get<int>("received"));
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
    Com(Stats * stats, GWHomie * h, WebSocket * ws) {
        this->swSer = new SoftwareSerial(SERIAL_PIN_1, SERIAL_PIN_2, false, 256);
        this->homie = h;
        this->ws = ws;
        this->stats = stats;
        this->statsChanged = false;
        stats->addChangeCallback([this](){
            String text = "Messages stats: Successfully sent=";
            text.concat(this->stats->getNowSentMessagesSuccessful());
            text.concat(" Failed=");
            text.concat(this->stats->getNowSentMessagesFailed());
            text.concat(" Received=");
            text.concat(this->stats->getNowSentMessagesReceived());
            this->ws->textAll((char *) text.c_str());
        });
    }
#endif
#if DEV_MODE == 2
    Com(Stats * stats) {
        swSer = new SoftwareSerial(SERIAL_PIN_2, SERIAL_PIN_1, false, 256);
        this->stats = stats;
        this->statsChanged = false;

        stats->addChangeCallback([this](){
            this->statsChanged = true;
        });
    }
#endif

    template <typename T1>
    void send(utils::msgType type, unsigned long id, String name1, T1 value1) {
        String s;
        send(type, id, name1, value1, s, s);
    }

    template <typename T1, typename T2>
    void send(utils::msgType type, unsigned long id, String name1, T1 value1, String name2, T2 value2) {
        String s;
        send(type, id, name1, value1, name2, value2, s, s);
    }

    template <typename T1, typename T2, typename T3>
    void send(utils::msgType type, unsigned long id, String name1, T1 value1, String name2, T2 value2, String name3, T3 value3) {
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
        if (name3.length() > 0) {
            json.set(name3, value3);
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
            bufferPosition = (uint8) (bufferPosition + 1) % 512;
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

#if DEV_MODE == 2
        if (statsChanged) {
            statsChanged = false;
            send(
                    utils::msgType::stats,
                    0,
                    "success",
                    stats->getNowSentMessagesSuccessful(),
                    "failed",
                    stats->getNowSentMessagesFailed(),
                    "received",
                    stats->getNowSentMessagesReceived()
            );
        }
#endif
    }

    void addOnNowMessageSendListener(onNowMessageSendCallback cb) {
        nowMessageSendCallbackVector.push_back(cb);
    }
};

#endif // COM_INIT_H
