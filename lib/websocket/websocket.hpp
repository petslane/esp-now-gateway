#ifndef WEBSOCKET_INIT_H
#define WEBSOCKET_INIT_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "utils.hpp"
#include "buffer.hpp"
#include "webserver.hpp"
#include <vector>

#define XSTR(s) STR(s)
#define STR(s) #s

#define WS_BUFFER_SIZE 10

namespace ws {
    AsyncWebSocket ws("/ws");

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
        }
    }

    void loop() {
        if (ptrIncomingMessages.size() > 0) {
            String * incomingMessage = ptrIncomingMessages.at(0);
            ptrIncomingMessages.erase(ptrIncomingMessages.begin());

            int len = incomingMessage->length();
            char raw[len + 1];
            strncpy(raw, incomingMessage->c_str(), (size_t) len);
            raw[len] = '\0';

            StaticJsonBuffer<200> jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(raw);


            if (!root.success()) {
                return;
            }
            if (!root.containsKey("type")) {
                return;
            }
            if (root.get<int>("type") == utils::msgType::send_now_message) {
                char mac[7];
                utils::macStringToCharArray(root.get<String>("to"), mac);

                String message = root.get<String>("message");
                unsigned long id = root.get<unsigned long>("id");
                Buffer::add_send_now_msg(mac, message.c_str(), message.length(), id);
            }

            delete incomingMessage;
        }
    }

    void setup() {
        ptrIncomingMessages.reserve(WS_BUFFER_SIZE);

        ws.onEvent(onEvent);
#ifdef AUTH_USERNAME
        ws.setAuthentication((const char* ) XSTR(AUTH_USERNAME), (const char* ) XSTR(AUTH_PASSWORD));
#endif

        webserver::server.addHandler(&ws);
    }

}

#endif // WEBSOCKET_INIT_H
