#ifndef STATS_INIT_H
#define STATS_INIT_H

#include <Base64.h>

class Stats {
private:
    struct {
        int volatile now_sent_messages_successful;
        int volatile now_sent_messages_failed;
        int volatile now_messages_received;
        int volatile incoming_buffer_size;
        int volatile incoming_buffer_free;
        int volatile messages_buffer_size;
        int volatile messages_buffer_free;
    } remoteData;

    struct {
    } localData;

    bool stats_updated;

    typedef std::function<void(void)> onStatsChangeCallback;
    std::vector<onStatsChangeCallback> onStatsChangeCallbackVector;
public:
    Stats() {
        remoteData.now_sent_messages_successful = 0;
        remoteData.now_sent_messages_failed = 0;
        remoteData.now_messages_received = 0;
        remoteData.incoming_buffer_size = 0;
        remoteData.incoming_buffer_free = 0;
        remoteData.messages_buffer_size = 0;
        remoteData.messages_buffer_free = 0;

        stats_updated = true;

        onStatsChangeCallbackVector.reserve(2);
    }

    void loop() {
        if (stats_updated) {
            stats_updated = false;

            std::for_each(onStatsChangeCallbackVector.begin(), onStatsChangeCallbackVector.end(), [](onStatsChangeCallback cb) {
                cb();
            });
        }
    }

    String packRemoteData() {
        char data[sizeof(remoteData)];
        memcpy(data, &remoteData, sizeof(remoteData));
        int base64length = Base64.encodedLength(sizeof(remoteData));
        char base64[base64length];
        Base64.encode(base64, data, sizeof(remoteData));

        return String(base64);
    }

    void unpackRemoteData(String base64Str) {
        int base64StrLen = base64Str.length();

        int decodedLen = Base64.decodedLength((char *) base64Str.c_str(), base64StrLen);

        char decodedString[decodedLen];
        Base64.decode(decodedString, (char *) base64Str.c_str(), base64StrLen);

        memcpy(&remoteData, decodedString, sizeof(remoteData));

        stats_updated = true;
    }

    void addChangeCallback(onStatsChangeCallback cb) {
        onStatsChangeCallbackVector.push_back(cb);
    }

    void addNowSentMessagesSuccessful(int value = 1) {
        stats_updated = true;
        remoteData.now_sent_messages_successful += value;
    }

    void addNowSentMessagesFailed(int value = 1) {
        stats_updated = true;
        remoteData.now_sent_messages_failed += value;
    }

    void addNowMessagesReceived(int value = 1) {
        stats_updated = true;
        remoteData.now_messages_received += value;
    }

    void addIncomingBufferFree(int value = 1) {
        stats_updated = true;
        remoteData.incoming_buffer_free += value;
    }

    void addMessageBufferFree(int value = 1) {
        stats_updated = true;
        remoteData.messages_buffer_free += value;
    }

    void setNowSentMessagesSuccessful(int value) {
        stats_updated = true;
        remoteData.now_sent_messages_successful = value;
    }

    void setNowSentMessagesFailed(int value) {
        stats_updated = true;
        remoteData.now_sent_messages_failed = value;
    }

    void setNowMessagesReceived(int value) {
        stats_updated = true;
        remoteData.now_messages_received = value;
    }

    void setIncomingBufferSize(int value) {
        stats_updated = true;
        remoteData.incoming_buffer_size = value;
    }

    void setIncomingBufferFree(int value) {
        stats_updated = true;
        remoteData.incoming_buffer_free = value;
    }

    void setMessageBufferSize(int value) {
        stats_updated = true;
        remoteData.messages_buffer_size = value;
    }

    void setMessageBufferFree(int value) {
        stats_updated = true;
        remoteData.messages_buffer_free = value;
    }

    int getNowSentMessagesSuccessful() {
        return remoteData.now_sent_messages_successful;
    }

    int getNowSentMessagesFailed() {
        return remoteData.now_sent_messages_failed;
    }

    int getNowMessagesReceived() {
        return remoteData.now_messages_received;
    }

    int getIncomingBufferSize() {
        return remoteData.incoming_buffer_size;
    }

    int getIncomingBufferFree() {
        return remoteData.incoming_buffer_free;
    }

    int getMessageBufferSize() {
        return remoteData.messages_buffer_size;
    }

    int getMessageBufferFree() {
        return remoteData.messages_buffer_free;
    }
};

#endif // STATS_INIT_H
