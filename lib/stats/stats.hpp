#ifndef STATS_INIT_H
#define STATS_INIT_H

class Stats {
private:
    int volatile now_sent_messages_successful;
    int volatile now_sent_messages_failed;
    int volatile now_messages_received;

    bool stats_updated;

    typedef std::function<void(void)> onStatsChangeCallback;
    std::vector<onStatsChangeCallback> onStatsChangeCallbackVector;
public:
    Stats() {
        now_sent_messages_successful = 0;
        now_sent_messages_failed = 0;
        now_messages_received = 0;

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

    void addChangeCallback(onStatsChangeCallback cb) {
        onStatsChangeCallbackVector.push_back(cb);
    }

    void addNowSentMessagesSuccessful(int value = 1) {
        stats_updated = true;
        now_sent_messages_successful += value;
    }

    void addNowSentMessagesFailed(int value = 1) {
        stats_updated = true;
        now_sent_messages_failed += value;
    }

    void addNowMessagesReceived(int value = 1) {
        stats_updated = true;
        now_messages_received += value;
    }

    void setNowSentMessagesSuccessful(int value) {
        stats_updated = true;
        now_sent_messages_successful = value;
    }

    void setNowSentMessagesFailed(int value) {
        stats_updated = true;
        now_sent_messages_failed = value;
    }

    void setNowMessagesReceived(int value) {
        stats_updated = true;
        now_messages_received = value;
    }

    int getNowSentMessagesSuccessful() {
        return now_sent_messages_successful;
    }

    int getNowSentMessagesFailed() {
        return now_sent_messages_failed;
    }

    int getNowMessagesReceived() {
        return now_messages_received;
    }
};

#endif // STATS_INIT_H
