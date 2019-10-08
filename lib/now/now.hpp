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

#define INCOMING_BUFFER_SIZE 20

uint8_t gatewayMac[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
//uint8_t gatewayMac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};

// is currently sending a message
static volatile bool is_sending = false;
// buffer index of message being sent
static int sending_message_header_index = 0;

struct IncomingNowMessage {
    bool set = false;
    NowMessageHeader header;
    char message[250];
};

// index of incomingBuffer where next value should be saved if free
volatile uint8 incomingBufferFreeSlot = 0;
// index of incomingBuffer (if set) that should be saved to now message buffer
volatile uint8 incomingBufferFilledSlot = 0;
// volatile buffer for accepting incoming NOW messages, from here messages move to `now_data_buffer`
volatile IncomingNowMessage incomingBuffer[INCOMING_BUFFER_SIZE];

// Now callback can not update class instance itself, so create global variables for stats to be synced with Stats instance
int volatile now_sent_messages_successful = 0;
int volatile now_sent_messages_failed = 0;
int volatile now_messages_received = 0;
int volatile missed_incoming_now_messages = 0;
int volatile incoming_buffer_free = INCOMING_BUFFER_SIZE;
int volatile message_buffer_free = NOW_BUFFER_SIZE;

class Now {
private:
    Com * com;
    Stats * stats;

    /**
     * Callback for NOW messages to be sent. Adds message to queue.
     */
    bool sendNowMessageOut(String mac, const char *msg, uint8 len, unsigned long id) {
        NowMessage nowMessage;
        if (!nowMessage.setMac(mac)) {
            return false;
        }
        nowMessage.buffer_data.msgLen = len;
        nowMessage.buffer_data.status = NowMessageStatus::queued;
        nowMessage.buffer_data.time = millis();
        nowMessage.buffer_data.id = id;
        nowMessage.buffer_data.errorCount = 0;

        bool added = nowMessage.append(msg, len) > -1;
        if (added) {
            message_buffer_free -= sizeof(NowMessageHeader);
            message_buffer_free -= len;
        }
        return added;
    }
public:
    Now(Com * c, Stats * stats) {
        this->com = c;
        this->com->addOnNowMessageSendListener(std::bind(&Now::sendNowMessageOut, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        this->stats = stats;
        this->stats->setIncomingBufferFree(INCOMING_BUFFER_SIZE);
        this->stats->setIncomingBufferSize(INCOMING_BUFFER_SIZE);
        this->stats->setMessageBufferFree(NOW_BUFFER_SIZE);
        this->stats->setMessageBufferSize(NOW_BUFFER_SIZE);
    }

    void loop() {
        // move `incomingBuffer` messages to `now_data_buffer`
        while (true) {
            if (!incomingBuffer[incomingBufferFilledSlot].set) {
                break;
            }
            // move incoming NOW message from `incomingBuffer` to `now_data_buffer`
            NowMessage msg;
            memcpy(& msg.buffer_data, (const void *) & incomingBuffer[incomingBufferFilledSlot].header, sizeof(NowMessageHeader));
            int added = msg.append((const char *) incomingBuffer[incomingBufferFilledSlot].message, msg.buffer_data.msgLen);
            if (added == -1) {
                break;
            }
            incomingBuffer[incomingBufferFilledSlot].set = false;
            incoming_buffer_free++;
            message_buffer_free -= sizeof(NowMessageHeader);
            message_buffer_free -= msg.buffer_data.msgLen;

            incomingBufferFilledSlot = (incomingBufferFilledSlot + (uint8) 1) % (uint8) INCOMING_BUFFER_SIZE;
        }

        int pos = 0;
        long loopStartTime = millis();
        uint8 previousIncomingBufferFreeSlot = incomingBufferFreeSlot;
        while (true) {
            if (previousIncomingBufferFreeSlot != incomingBufferFreeSlot || millis() - loopStartTime > 500) {
                break;
            }
            NowMessage msg(pos);
            if (!msg.isSaved()) {
                break;
            }

            if (msg.buffer_data.status == NowMessageStatus::sent || msg.buffer_data.status == NowMessageStatus::failed) {
                String mac = utils::macCharArrayToString(msg.buffer_data.mac);
                utils::msgType type = msg.buffer_data.status == NowMessageStatus::sent ? utils::msgType::now_message_delivered : utils::msgType::now_message_not_delivered;
                this->com->send(type, msg.buffer_data.id, "to", mac, "msg", msg.getMessage());
                message_buffer_free += sizeof(NowMessageHeader);
                message_buffer_free += msg.buffer_data.msgLen;
                msg.remove();
                if (is_sending && pos < sending_message_header_index) {
                    sending_message_header_index--;
                }
                continue;
            }

            if (msg.buffer_data.status == NowMessageStatus::received) {
                String mac = utils::macCharArrayToString(msg.buffer_data.mac);
                this->com->send(utils::msgType::received_now_message, msg.buffer_data.id, "from", mac, "msg", msg.getMessage());
                message_buffer_free += sizeof(NowMessageHeader);
                message_buffer_free += msg.buffer_data.msgLen;
                msg.remove();
                if (is_sending && pos < sending_message_header_index) {
                    sending_message_header_index--;
                }
                continue;
            }

            if (msg.buffer_data.status == NowMessageStatus::delayed && (signed long) (msg.buffer_data.time - millis()) < 0) {
                msg.buffer_data.status = NowMessageStatus::queued;
            }

            if (msg.buffer_data.status == NowMessageStatus::queued && (signed long) (msg.buffer_data.time - millis()) < 0 && !is_sending) {
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

        if (now_sent_messages_successful) {
            this->stats->addNowSentMessagesSuccessful(now_sent_messages_successful);
            now_sent_messages_successful = 0;
        }
        if (now_sent_messages_failed) {
            this->stats->addNowSentMessagesFailed(now_sent_messages_failed);
            now_sent_messages_failed = 0;
        }
        if (now_messages_received) {
            this->stats->addNowMessagesReceived(now_messages_received);
            now_messages_received = 0;
        }
        if (missed_incoming_now_messages) {
            this->stats->addMissedIncomingNowMessages(missed_incoming_now_messages);
            missed_incoming_now_messages = 0;
        }
        if (incoming_buffer_free != this->stats->getIncomingBufferFree()) {
            this->stats->setIncomingBufferFree(incoming_buffer_free);
        }
        if (message_buffer_free != this->stats->getMessageBufferFree()) {
            this->stats->setMessageBufferFree(message_buffer_free);
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
            if (incomingBuffer[incomingBufferFreeSlot].set) {
                // No free slots for accepting incoming NOW message, discarding message
                missed_incoming_now_messages++;
                return;
            }

            // save incoming message to buffer
            memcpy((void *) incomingBuffer[incomingBufferFreeSlot].header.mac, mac, 6);
            incomingBuffer[incomingBufferFreeSlot].header.msgLen = len;
            incomingBuffer[incomingBufferFreeSlot].header.status = NowMessageStatus::received;
            incomingBuffer[incomingBufferFreeSlot].header.time = millis();
            incomingBuffer[incomingBufferFreeSlot].header.id = millis();
            incomingBuffer[incomingBufferFreeSlot].header.errorCount = 0;
            memcpy((void *) incomingBuffer[incomingBufferFreeSlot].message, (char *) data, len);
            incomingBuffer[incomingBufferFreeSlot].set = true;
            incoming_buffer_free--;
            incomingBufferFreeSlot = (incomingBufferFreeSlot + (uint8) 1) % (uint8) INCOMING_BUFFER_SIZE;

            now_messages_received++;
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
