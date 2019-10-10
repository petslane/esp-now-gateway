#pragma once

#include <Arduino.h>

#define NOW_BUFFER_SIZE 20000

static volatile char now_data_buffer[NOW_BUFFER_SIZE];

enum NowMessageStatus: uint8 {
    queued = 0, // message ready to be sent
    sending = 1, // message sent, waiting response status
    sent = 2, // message sent successfully
    failed = 3, // message sending failed
    delayed = 4, // message sending delayed, can be because of sending failed
    received = 5, // new now message received from remote
};

struct NowMessageHeader {
    char mac[6];
    uint8 msgLen;
    unsigned long time;
    NowMessageStatus status;
    unsigned long id;
    uint8 errorCount;
};

class NowMessage {
    bool in_buffer = false;
    int data_index = -1;
public:
    NowMessageHeader buffer_data;
    NowMessage ();
    NowMessage (int data_index);
    String getMessage();
    int append();
    int append(const char * msg, uint8 msgLen);
    int hasNext();
    NowMessage * getNext();
    bool isSaved();
    bool setMac(String mac);
    void setStatus (NowMessageStatus status);
    void remove();
};


NowMessage::NowMessage() {
    in_buffer = false;
}
NowMessage::NowMessage(int data_index) {
    in_buffer = true;
    memcpy(& buffer_data, (const void *) & now_data_buffer[data_index], sizeof(NowMessageHeader));
    if (memcmp(buffer_data.mac, "\0\0\0\0\0\0", 6) == 0) {
        in_buffer = false;
    } else {
        this->data_index = data_index;
    }
}
String NowMessage::getMessage() {
    uint8 len = this->buffer_data.msgLen;
    if (len <= 0) {
        return "";
    }

    char buf[len + 1];
    memcpy(buf, (const void *) & now_data_buffer[this->data_index + sizeof(NowMessageHeader)], (size_t) len);
    buf[len] = '\0';

    return String(buf);
}
int NowMessage::append() {
    return append("", 0);
}
int NowMessage::append(const char * msg, uint8 msgLen) {
    if (in_buffer) {
        return -1;
    }

    int i = 0;
    while (true) {
        if (i + sizeof(NowMessageHeader) + msgLen >= NOW_BUFFER_SIZE) {
            return -1;
        }
        NowMessageHeader header;
        memcpy(& header, (const void *) & now_data_buffer[i], sizeof(NowMessageHeader));
        if (memcmp(header.mac, "\0\0\0\0\0\0", 6) == 0) {
            break;
        }
        i += sizeof(NowMessageHeader) + header.msgLen;
    }
    data_index = i;

    buffer_data.msgLen = msgLen;
    memcpy((void *) & now_data_buffer[data_index], & buffer_data, sizeof(NowMessageHeader));
    if (msgLen > 0) {
        memcpy((void *) & now_data_buffer[data_index + sizeof(NowMessageHeader)], & msg[0], msgLen);
    }
    in_buffer = true;

    return data_index;
}
int NowMessage::hasNext() {
    if (!in_buffer) {
        return -1;
    }
    size_t nextPosition = data_index + sizeof(NowMessageHeader) + buffer_data.msgLen;
    if (nextPosition + sizeof(NowMessageHeader) >= NOW_BUFFER_SIZE) {
        return -1;
    }
    NowMessageHeader header;
    memcpy(& header, (const void *) & now_data_buffer[nextPosition], sizeof(NowMessageHeader));
    if (memcmp(header.mac, "\0\0\0\0\0\0", 6) == 0) {
        return -1;
    }
    return nextPosition;
}
NowMessage * NowMessage::getNext() {
    int nextPos = hasNext();
    if (nextPos < 0) {
        return new NowMessage();
    }
    return new NowMessage(nextPos);
}
bool NowMessage::isSaved() {
    return in_buffer;
}
bool NowMessage::setMac(String mac) {
    return utils::macStringToCharArray(mac, this->buffer_data.mac);
}
void NowMessage::setStatus (NowMessageStatus status) {
    buffer_data.status = status;
    if (!in_buffer) {
        return;
    }
    memcpy((void *) & now_data_buffer[data_index], & buffer_data, sizeof(NowMessageHeader));
}
void NowMessage::remove() {
    if (!in_buffer) {
        return;
    }
    int dataLen = sizeof(NowMessageHeader) + buffer_data.msgLen;

    memmove((void *) & now_data_buffer[data_index], (const void *) & now_data_buffer[data_index + dataLen], NOW_BUFFER_SIZE - data_index - dataLen);
    memset((void *) & now_data_buffer[NOW_BUFFER_SIZE - dataLen], '\0', dataLen);

    in_buffer = false;
}
