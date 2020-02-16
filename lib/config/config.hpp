#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

struct ConfigStruct {
    struct {
        String host;
        int port;
        String baseTopic;
    } mqtt;

    struct {
        String ssid;
        String pass;
    } wifi;

    struct {
        String name;
    } ap;

    struct {
        char mac[7];
        bool changed; // this value is not saved to JSON
    } now;
};

ConfigStruct config;

class Config {
  private:
    static void error(String message) {
        Serial.println(message);
        delay(1000);
        ESP.reset();
    }

    static String setJson(JsonDocument *doc) {
        SPIFFS.begin();
        String filename = "/config.json";

        File f = SPIFFS.open(filename, "w");
        if (!f) {
            SPIFFS.end();
            return "unable to open config.json file";
        }

        serializeJsonPretty(*doc, f);
        f.close();
        SPIFFS.end();

        return "";
    }

    static String getJson(JsonDocument *doc) {
        SPIFFS.begin();
        String filename = "/config.json";

        if (!SPIFFS.exists(filename)) {
            SPIFFS.end();
            return "config.json file missing";
        }

        File f = SPIFFS.open(filename, "r");
        if (!f) {
            SPIFFS.end();
            return "unable to open config.json file";
        }

        Serial.println("config.json content:");
        deserializeJson(*doc, Serial);
        Serial.println();

        DeserializationError err = deserializeJson(*doc, f);
        f.close();
        SPIFFS.end();

        if (err != DeserializationError::Ok) {
            return "Invalid config.json content";
        }

        return "";
    }

  public:
    static void load() {
        DynamicJsonDocument doc(500);
        String errorMessage = getJson(&doc);

        if (errorMessage.length()) {
            error(errorMessage);
            return;
        }

        JsonObject root = doc.as<JsonObject>();

        JsonObject wifi = root["wifi"];
        config.wifi.ssid = wifi["ssid"].as<String>();
        config.wifi.pass = wifi["password"].as<String>();

        JsonObject mqtt = root["mqtt"];
        config.mqtt.host = mqtt["host"].as<String>();
        config.mqtt.port = mqtt["port"].as<int>();
        config.mqtt.baseTopic = mqtt["base_topic"].as<String>();

        JsonObject ap = root["ap"];
        config.ap.name = ap["name"].as<String>();

        JsonObject now = root["now"];
        String nowMac = now["mac"].as<String>();
        if (!utils::macStringToCharArray(nowMac, config.now.mac) || memcmp(config.now.mac, "\0\0\0\0\0\0", 6) == 0) {
            utils::macStringToCharArray("30:30:30:30:30:30", config.now.mac);
        }
        config.now.changed = true;
    }

    static String saveConfig() {
        DynamicJsonDocument doc(500);
        String errorMessage = getJson(&doc);

        if (errorMessage.length()) {
            return errorMessage;
        }

        JsonObject root = doc.as<JsonObject>();

        JsonObject wifi = root["wifi"];
        wifi["ssid"] = config.wifi.ssid;
        wifi["password"] = config.wifi.pass;

        JsonObject mqtt = root["mqtt"];
        mqtt["host"] = config.mqtt.host;
        mqtt["port"] = config.mqtt.port;
        mqtt["base_topic"] = config.mqtt.baseTopic;

        JsonObject ap = root["ap"];
        ap["name"] = config.ap.name;

        if (!root.containsKey("now")) {
            root.createNestedObject("now");
        }
        JsonObject now = root["now"];
        now["mac"] = utils::macCharArrayToString(config.now.mac);

        return setJson(&doc);
    }

    static String saveDevice(String mac, String name) {
        DynamicJsonDocument doc(500);
        String errorMessage = getJson(&doc);

        if (errorMessage.length()) {
            return errorMessage;
        }

        JsonObject root = doc.as<JsonObject>();
        if (!root.containsKey("devices")) {
            root.createNestedObject("devices");
        }
        JsonObject devices = root["devices"];

        if (devices.containsKey(mac)) {
            devices.remove(mac);
        }
        if (name.length()) {
            devices[mac] = name;
        }

        serializeJson(doc, Serial);

        return setJson(&doc);
    }
};
