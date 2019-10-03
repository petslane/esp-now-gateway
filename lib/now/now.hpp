#ifndef NOW_INIT_H
#define NOW_INIT_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "utils.hpp"
#include "now_message.hpp"
#include "com.hpp"
#include "stats.hpp"

extern "C" {
#include <espnow.h>
#include "user_interface.h"
}

#define INCOMING_BUFFER_SIZE 10

uint8_t gatewayMac[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
//uint8_t gatewayMac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};

static volatile bool is_sending = false;
static int sending_message_header_index = 0;

struct IncomingNowMessage {
    bool set = false;
    NowMessageHeader header;
    char message[250];
};

static volatile IncomingNowMessage incomingBuffer[INCOMING_BUFFER_SIZE];

// Now callback can not update class instance itself, so create global variables for stats to be synced with Stats instance
int volatile now_sent_messages_successful = 0;
int volatile now_sent_messages_failed = 0;
int volatile now_sent_messages_received = 0;

class Now {
private:
    Com * com;
    Stats * stats;
public:
    Now(Com * c, Stats * stats) {
        this->com = c;
        this->com->addOnNowMessageSendListener(std::bind(&Now::send, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        this->stats = stats;
    }

    bool send(String mac, const char * msg, uint8 len, unsigned long id) {
        NowMessage nowMessage;
        if (!nowMessage.setMac(mac)) {
            return false;
        }
        nowMessage.buffer_data.msgLen = len;
        nowMessage.buffer_data.status = NowMessageStatus::queued;
        nowMessage.buffer_data.time = millis();
        nowMessage.buffer_data.id = id;
        nowMessage.buffer_data.errorCount = 0;
        return nowMessage.append(msg, len) > -1;
    }

    void loop() {
        for (uint8 i=0; i < INCOMING_BUFFER_SIZE; i++) {
            if (incomingBuffer[i].set) {
                NowMessage msg;
                memcpy(& msg.buffer_data, (const void *) & incomingBuffer[i].header, sizeof(NowMessageHeader));
                msg.append((const char *) incomingBuffer[i].message, msg.buffer_data.msgLen);
                incomingBuffer[i].set = false;
            }
        }

        if (!is_sending) {
            int pos = 0;
            while (true) {
                NowMessage msg(pos);
                if (!msg.isSaved()) {
                    break;
                }

                if (msg.buffer_data.status == NowMessageStatus::sent || msg.buffer_data.status == NowMessageStatus::failed) {
                    String mac = utils::macCharArrayToString(msg.buffer_data.mac);
                    utils::msgType type = msg.buffer_data.status == NowMessageStatus::sent ? utils::msgType::now_message_delivered : utils::msgType::now_message_not_delivered;
                    this->com->send(type, msg.buffer_data.id, "to", mac, "msg", msg.getMessage());
                    msg.remove();
                    continue;
                }

                if (msg.buffer_data.status == NowMessageStatus::received) {
                    String mac = utils::macCharArrayToString(msg.buffer_data.mac);
                    this->com->send(utils::msgType::received_now_message, msg.buffer_data.id, "from", mac, "msg", msg.getMessage());
                    msg.remove();
                    continue;
                }

                if (msg.buffer_data.status == NowMessageStatus::delayed && (signed long) (msg.buffer_data.time - millis()) < 0) {
                    msg.buffer_data.status = NowMessageStatus::queued;
                }

                if (msg.buffer_data.status == NowMessageStatus::queued && (signed long) (msg.buffer_data.time - millis()) < 0) {
                    is_sending = true;
                    sending_message_header_index = pos;

                    Serial.printf("[now] Sending NOW message %02X:%02X:%02X:%02X:%02X:%02X: %s\n", msg.buffer_data.mac[0], msg.buffer_data.mac[1], msg.buffer_data.mac[2], msg.buffer_data.mac[3], msg.buffer_data.mac[4], msg.buffer_data.mac[5], msg.getMessage().c_str());

                    msg.setStatus(NowMessageStatus::sending);

                    String message = msg.getMessage();
                    esp_now_send((u8 *) msg.buffer_data.mac, (u8 *) message.c_str(), message.length());
                    break;
                }

                pos = msg.hasNext();
                if (pos < 0) {
                    break;
                }
            }
        }

        while (now_sent_messages_successful) {
            this->stats->addNowSentMessagesSuccessful();
            now_sent_messages_successful--;
        }
        while (now_sent_messages_failed) {
            this->stats->addNowSentMessagesFailed();
            now_sent_messages_failed--;
        }
        while (now_sent_messages_received) {
            this->stats->addNowSentMessagesReceived();
            now_sent_messages_received--;
        }
    }

    void setup() {
        memset((void *) now_data_buffer, '\0', NOW_BUFFER_SIZE);

        WiFi.mode(WIFI_AP);
        wifi_set_macaddr(SOFTAP_IF, gatewayMac);

        if (esp_now_init() != 0) {
            ESP.restart();
        }

        esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

        // receiving now
        esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {
            char raw[len + 1];
            memcpy((void *) raw, (char *) data, len);
            Serial.printf("[now] Incoming NOW message from %02X:%02X:%02X:%02X:%02X:%02X: %s\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], raw);
            for (uint8 i=0; i < INCOMING_BUFFER_SIZE; i++) {
                if (!incomingBuffer[i].set) {
                    memcpy((void *) incomingBuffer[i].header.mac, mac, 6);
                    incomingBuffer[i].header.msgLen = len;
                    incomingBuffer[i].header.status = NowMessageStatus::received;
                    incomingBuffer[i].header.time = millis();
                    incomingBuffer[i].header.id = millis();
                    incomingBuffer[i].header.errorCount = 0;
                    memcpy((void *) incomingBuffer[i].message, (char *) data, len);
                    incomingBuffer[i].set = true;
                    now_sent_messages_received++;
                    return;
                }
            }

            Serial.println("[now] !!!!!!!!!!!!!!!! Received message, but previous message still in buffer");
        });

        // sent now cb
        esp_now_register_send_cb([](uint8_t *mac, uint8_t sendStatus) {
            Serial.printf("[now] Sending NOW message to %02X:%02X:%02X:%02X:%02X:%02X %s\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], sendStatus ? "failed" : "succeed");

            if (!is_sending) {
                return;
            }

            NowMessage msg(sending_message_header_index);
            if (!msg.isSaved()) {
                return;
            }
            if (memcmp(msg.buffer_data.mac, mac, 6) != 0) {
                return;
            }

            if (sendStatus != 0) { // sending failed
                msg.buffer_data.time += 5000;
                msg.buffer_data.errorCount += 1;

                msg.setStatus(msg.buffer_data.errorCount > 3 ? NowMessageStatus::failed : NowMessageStatus::delayed);
                if (msg.buffer_data.errorCount > 3) {
                    now_sent_messages_failed++;
                }
            } else {
                msg.setStatus(NowMessageStatus::sent);
                now_sent_messages_successful++;
            }

            is_sending = false;
        });
    }
};

#endif // NOW_INIT_H
