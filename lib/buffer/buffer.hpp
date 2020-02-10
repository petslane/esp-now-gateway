#pragma once

#include <Arduino.h>
#include <utils.hpp>

#define BUFFER_DATA_SIZE 1000

namespace Buffer {

    struct Index {
        utils::msgType type;
        unsigned long id;
        uint8 msgLen;
        union {
            // send now message
            struct {
                char mac[6];
            } send_now_message;
        } attr;
    };
    static char data[BUFFER_DATA_SIZE];

    Index &get_index(int buffer_index) {
        return (Index &)*(data + buffer_index);
    }
    Index &get_index_by_index(int index) {
        int buffer_index = 0;
        for (int i = 0; i < index; i++) {
            int new_buffer_index = buffer_index + sizeof(Index) + ((Index *)(data + buffer_index))->msgLen;
            if (new_buffer_index >= BUFFER_DATA_SIZE) {
                break;
            }
            buffer_index = new_buffer_index;
        }
        return get_index(buffer_index);
    }

    int get_free_buffer_index() {
        uint16 data_pos = 0;

        while (true) {
            Index index = get_index(data_pos);

            if (index.type == utils::msgType::undefined) {
                return data_pos;
            }
            data_pos += sizeof(Index) + index.msgLen;
            if (data_pos + sizeof(Index) >= BUFFER_DATA_SIZE) {
                return -1;
            }
        }
    }

    uint16 get_buffer_free_space() {
        int used = get_free_buffer_index();
        if (used < 0) {
            return 0;
        }
        return (uint16)BUFFER_DATA_SIZE - (uint16)used;
    }

    bool is_buffer_empty() {
        return get_index(0).type == utils::msgType::undefined;
    }

    String get_data(uint8 index) {
        int data_start = 0;
        int data_len = 0;
        for (int i = 0; i < index; i++) {
            Index idx = get_index(data_start);
            data_start += sizeof(Index) + idx.msgLen;
        }
        Index idx = get_index(data_start);
        data_start += sizeof(Index);
        data_len = idx.msgLen;

        if (data_len == 0) {
            return "";
        }

        char buf[data_len + 1];
        memcpy(buf, data + data_start, (size_t)data_len);
        buf[data_len] = '\0';

        return String(buf);
    }

    void remove(uint8 index) {
        int data_start = 0;
        int data_len = 0;
        for (int i = 0; i < index; i++) {
            Index idx = get_index(data_start);
            data_start += sizeof(Index) + idx.msgLen;
        }
        Index idx = get_index(data_start);
        data_len = sizeof(Index) + idx.msgLen;

        memmove(&data[data_start], &data[data_start + data_len], BUFFER_DATA_SIZE - data_start - data_len);
        memset(&data[BUFFER_DATA_SIZE - data_len], '\0', data_len);
    }

    bool append(Index index, const char *message, int message_len) {
        if (message_len > 250) {
            return false;
        }

        size_t index_len = sizeof(Index);
        if (index_len + message_len >= get_buffer_free_space()) {
            return false;
        }

        int free_index = get_free_buffer_index();

        memcpy(&data[free_index], &index, index_len);
        if (message_len > 0) {
            memcpy(&data[free_index + index_len], &message[0], (size_t)message_len);
        }

        return true;
    }

    bool append(Index index) {
        return append(index, "", 0);
    }

    bool add_send_now_msg(const char *mac, const char *message, int len, unsigned long id) {
        Index index;
        index.type = utils::msgType::send_now_message;
        index.id = id;
        memcpy(index.attr.send_now_message.mac, mac, 6);
        index.msgLen = (uint8)len;
        return append(index, message, len);
    }

    void setup() {
        for (int i = 0; i < BUFFER_DATA_SIZE; i++) {
            data[i] = '\0';
        }
    }
} // namespace Buffer
