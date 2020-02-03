#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <utils.hpp>
#include <buffer.hpp>
#include <webserver.hpp>
#include <vector>
#include <stats.hpp>
#include <ArduinoJson.h>

#define XSTR(s) STR(s)
#define STR(s) #s

#define WS_BUFFER_SIZE 10

class WebSocket {
private:
    AsyncWebSocket * ws;
    WebServer * webserver;
    Stats * stats;

    bool statsChanged = false;
    long lastStatsReportTime = 0;

    std::vector<String*> ptrIncomingMessages;

    void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
        //Handle WebSocket event
        if (type == AwsEventType::WS_EVT_DATA ) {
            AwsFrameInfo * info = (AwsFrameInfo*)arg;
            if(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
                char dataWithNull[len + 1];
                memcpy(dataWithNull, data, len);
                dataWithNull[len] = '\0';

                String * incomingData = new String(dataWithNull);
                ptrIncomingMessages.push_back(incomingData);
            }
        } else if (type == AwsEventType::WS_EVT_CONNECT) {
            statsChanged = true;
        }
    }

    void publishStats() {
        StaticJsonDocument<200> doc;
        doc["type"] = "stats";
        doc["buffer1_free"] = stats->getIncomingBufferFree();
        doc["buffer1_size"] = stats->getIncomingBufferSize();
        doc["buffer2_free"] = stats->getMessageBufferFree();
        doc["buffer2_size"] = stats->getMessageBufferSize();
        doc["missed_incoming_messages"] = stats->getMissedIncomingNowMessages();
        doc["msgs_failed_sent"] = stats->getNowSentMessagesFailed();
        doc["msgs_received"] = stats->getNowMessagesReceived();
        doc["msgs_success_sent"] = stats->getNowSentMessagesSuccessful();

        String msg;
        serializeJson(doc, msg);

        textAll(msg.c_str());
    }

public:
    WebSocket(WebServer * webserver, Stats * stats) {
        this->ws = new AsyncWebSocket("/ws");
        this->webserver = webserver;
        this->stats = stats;

        this->statsChanged = false;
        stats->addChangeCallback([this](){
            this->statsChanged = true;
        });
    }
    void loop() {
        if (ptrIncomingMessages.size() > 0) {
            String * incomingMessage = ptrIncomingMessages.at(0);
            ptrIncomingMessages.erase(ptrIncomingMessages.begin());

            int len = incomingMessage->length();
            char raw[len + 1];
            strncpy(raw, incomingMessage->c_str(), (size_t) len);
            raw[len] = '\0';

            StaticJsonDocument<200> doc;
            DeserializationError err = deserializeJson(doc, raw);

            if (err != DeserializationError::Ok) {
                return;
            }

            JsonObject root = doc.as<JsonObject>();
            if (!root.containsKey("type")) {
                return;
            }
            if (root["type"].as<int>() == utils::msgType::send_now_message) {
                char mac[7];
                utils::macStringToCharArray(root["to"].as<String>(), mac);

                String message = root["message"].as<String>();
                unsigned long id = root["id"].as<unsigned long>();
                Buffer::add_send_now_msg(mac, message.c_str(), message.length(), id);
            }

            delete incomingMessage;
        }

        if (statsChanged && millis() - lastStatsReportTime > 300) {
            lastStatsReportTime = millis();
            statsChanged = false;

            publishStats();
        }
    }

    void setup() {
        ptrIncomingMessages.reserve(WS_BUFFER_SIZE);

        ws->onEvent(std::bind(
                &WebSocket::onEvent,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4,
                std::placeholders::_5,
                std::placeholders::_6
        ));
#ifdef AUTH_USERNAME
        ws->setAuthentication((const char* ) XSTR(AUTH_USERNAME), (const char* ) XSTR(AUTH_PASSWORD));
#endif

        webserver->addHandler(ws);
    }

    void textAll(const char * msg) {
        ws->textAll(msg);
    }

};
