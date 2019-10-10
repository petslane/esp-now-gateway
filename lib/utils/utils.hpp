#pragma once

#include <Arduino.h>
#include "ArduinoJson.h"

namespace utils {
    enum msgType: uint8 {
        undefined = 0,
        send_now_message = 1,
        now_message_delivered = 2,
        now_message_not_delivered = 3,
        received_now_message = 4,
        stats = 5,
    };

    String macCharArrayToString(char * mac) {
        char macAddr[18];
        sprintf(macAddr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return macAddr;
    }

    bool macStringToCharArray(String from, char * to) {
        int mac[6];
        int i = sscanf(from.c_str(), "%x:%x:%x:%x:%x:%x%*c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        if (i != 6) {
            return false;
        }

        for (uint8 i = 0; i < 6; i++) {
            to[i] = (char) mac[i];
        }

        return true;
    }

    void mergeJson(JsonObject& dest, JsonObject& src) {
        for (auto kvp : src) {
            dest[kvp.key] = kvp.value;
        }
    }
}
