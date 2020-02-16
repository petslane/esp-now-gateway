#pragma once

#define XSTR(s) STR(s)
#define STR(s) #s

#include <./captive.hpp>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <config.hpp>
#include <utils.hpp>
#include <wifi.hpp>

class WebServer {
  private:
    AsyncWebServer *server;
    WIFI *wifi;

    void notFound(AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    }

    bool authenticateRequest(AsyncWebServerRequest *request) {
#ifndef AUTH_USERNAME
        return true;
#endif
        if (!request->authenticate((const char *)XSTR(AUTH_USERNAME), (const char *)XSTR(AUTH_PASSWORD))) {
            Serial.println("[web] Invalid username/password");
            request->requestAuthentication();
            return false;
        }

        return true;
    }

    bool readDevicesJson(String *out) {
        SPIFFS.begin();

        if (!SPIFFS.exists("/config.json")) {
            *out = "config.json file missing";
            SPIFFS.end();
            return false;
        }

        File f = SPIFFS.open("/config.json", "r");
        if (!f) {
            *out = "unable to open config.json file";
            SPIFFS.end();
            return false;
        }

        StaticJsonDocument<500> doc;
        DeserializationError err = deserializeJson(doc, f);
        f.close();
        SPIFFS.end();

        if (err != DeserializationError::Ok) {
            *out = "Invalid config.json content";
            return false;
        }

        JsonObject root = doc.as<JsonObject>();

        DynamicJsonDocument doc2(200);
        JsonObject obj = doc2.to<JsonObject>();

        utils::mergeJson(obj, root["devices"]);

        serializeJson(doc2, *out);

        return true;
    }

    void apiWifiConnect(AsyncWebServerRequest *request, AsyncJsonResponse *response, JsonObject root, String ssid,
                        String pass) {
        root["status"] = true;

        wifi->testWifiConnection(ssid, pass);
    }

    void apiGetWifiSettings(AsyncWebServerRequest *request, AsyncJsonResponse *response, JsonObject root) {
        root["status"] = true;
        root["current"] = WiFi.SSID();
        station_status_t status = wifi_station_get_connect_status();
        root["current_connected"] = status == STATION_GOT_IP;
        if (status == STATION_NO_AP_FOUND) {
            root["current_error"] = "No AP";
        } else if (status == STATION_CONNECT_FAIL) {
            root["current_error"] = "Connect fail";
        } else if (status == STATION_WRONG_PASSWORD) {
            root["current_error"] = "Wrong password";
        } else if (status == STATION_IDLE) {
            root["current_error"] = "Connecting";
        }
        root["ap_name"] = config.ap.name;
        root["now_mac"] = utils::macCharArrayToString(config.now.mac);
    }

    void apiSaveApName(AsyncWebServerRequest *request, AsyncJsonResponse *response, JsonObject root, String name) {
        config.ap.name = name;
        Config::saveConfig();

        apiGetWifiSettings(request, response, root);
    }

    void apiScanWifiNetworks(AsyncWebServerRequest *request, AsyncJsonResponse *response, JsonObject root) {
        WiFi.scanDelete();
        WiFi.scanNetworks(true);

        root["status"] = true;
    }

    void apiGetWifiNetworks(AsyncWebServerRequest *request, AsyncJsonResponse *response, JsonObject root) {
        JsonArray data = root.createNestedArray("data");
        StaticJsonDocument<200> doc;

        int n = WiFi.scanComplete();
        if (n >= 0) {
            for (int i = 0; i < n; i++) {
                Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n",
                              i + 1,
                              WiFi.SSID(i).c_str(),
                              WiFi.channel(i),
                              WiFi.RSSI(i),
                              WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");

                JsonArray network = doc.to<JsonArray>();
                network.add(WiFi.SSID(i));
                network.add(WiFi.channel(i));
                network.add(WiFi.RSSI(i));
                network.add(WiFi.encryptionType(i) != ENC_TYPE_NONE);
                data.add(network);
            }
            WiFi.scanDelete();
        }
        root["scanning"] = n < 0;
        root["current"] = WiFi.SSID();
        station_status_t status = wifi_station_get_connect_status();
        root["current_connected"] = status == STATION_GOT_IP;
        if (status == STATION_NO_AP_FOUND) {
            root["current_error"] = "No AP";
        } else if (status == STATION_CONNECT_FAIL) {
            root["current_error"] = "Connect fail";
        } else if (status == STATION_WRONG_PASSWORD) {
            root["current_error"] = "Wrong password";
        } else if (status == STATION_IDLE) {
            root["current_error"] = "Connecting";
        }

        root["status"] = true;
    }

    void apiGetDevices(AsyncWebServerRequest *request, AsyncJsonResponse *response, JsonObject root) {
        StaticJsonDocument<200> doc;
        String raw;

        root["status"] = false;

        if (!readDevicesJson(&raw)) {
            root["error"] = raw;
            return;
        }

        deserializeJson(doc, raw);

        JsonObject fileRoot = doc.as<JsonObject>();

        root["status"] = true;
        root.createNestedObject("data");
        utils::mergeJson(root["data"], fileRoot);
    }

    void apiDeleteDevice(AsyncWebServerRequest *request, AsyncJsonResponse *response, JsonObject root, String id) {
        String error = Config::saveDevice(id, "");

        root["status"] = false;
        if (error.length()) {
            root["error"] = error;
            return;
        }

        apiGetDevices(request, response, root);
    }

    void apiSaveDevice(AsyncWebServerRequest *request, AsyncJsonResponse *response, JsonObject root, String id,
                       String name, String mac) {

        String error = Config::saveDevice(mac, name);

        root["status"] = false;
        if (error.length()) {
            root["error"] = error;
            return;
        }

        apiGetDevices(request, response, root);
    }

    void apiSaveNowMac(AsyncWebServerRequest *request, AsyncJsonResponse *response, JsonObject root, String mac) {
        root["status"] = false;

        char mac_c[7];
        if (!utils::macStringToCharArray(mac, mac_c)) {
            root["error"] = "Invalid MAC address value";
            return;
        }

        memcpy(config.now.mac, mac_c, 6);
        Config::saveConfig();
        config.now.changed = true;

        root["status"] = true;
        root["now_mac"] = utils::macCharArrayToString(config.now.mac);
    }

    bool apiRequireParam(JsonObject &jsonObj, String param, AsyncWebServerRequest *request) {
        if (!jsonObj.containsKey(param)) {
            request->send(400, "text/plain", "Missing parameter " + param);
            return false;
        }

        return true;
    }

  public:
    WebServer(WIFI *wifi) {
        this->server = new AsyncWebServer(80);
        this->wifi = wifi;
    }

    void addHandler(AsyncWebHandler *handler) {
        this->server->addHandler(handler);
    }

    void setCORSHeaders(AsyncWebServerRequest *request, AsyncWebServerResponse *response) {
        response->addHeader("Access-Control-Allow-Headers", "Content-Type");
        AsyncWebHeader *origin = request->getHeader("Origin");
        response->addHeader("Access-Control-Allow-Origin", origin->value());
        response->addHeader("Access-Control-Allow-Credentials", "true");
    }

    void setup() {
        server->addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP

        server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
            if (!authenticateRequest(request)) {
                return;
            }

            SPIFFS.begin();
            AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/www/index.html.gz", "text/html");
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
            // SPIFFS.end();
        });
        server->on("/app.js", HTTP_GET, [this](AsyncWebServerRequest *request) {
            if (!authenticateRequest(request)) {
                return;
            }

            SPIFFS.begin();
            AsyncWebServerResponse *response =
                request->beginResponse(SPIFFS, "/www/app.js.gz", "application/javascript");
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
            // SPIFFS.end();
        });

        server->on("/api", HTTP_OPTIONS, [this](AsyncWebServerRequest *request) {
            AsyncWebServerResponse *response = request->beginResponse(200);

            setCORSHeaders(request, response);

            request->send(response);
        });

        server->on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });

        server->on("/fwlink", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });

        AsyncCallbackJsonWebHandler *handler =
            new AsyncCallbackJsonWebHandler("/api", [this](AsyncWebServerRequest *request, JsonVariant &json) {
                if (!authenticateRequest(request)) {
                    return;
                }

                JsonObject jsonObj = json.as<JsonObject>();

                if (!apiRequireParam(jsonObj, "type", request)) {
                    return;
                }

                String type = jsonObj["type"].as<String>();

                std::vector<String> requiredKeys;
                if (type.equals("get_devices")) {

                } else if (type.equals("delete_device")) {
                    requiredKeys.emplace_back("id");
                } else if (type.equals("save_device")) {
                    requiredKeys.emplace_back("name");
                    requiredKeys.emplace_back("mac");
                } else if (type.equals("connect_wifi")) {
                    requiredKeys.emplace_back("ssid");
                    requiredKeys.emplace_back("pass");
                } else if (type.equals("save_ap_name")) {
                    requiredKeys.emplace_back("name");
                } else if (type.equals("save_now_mac")) {
                    requiredKeys.emplace_back("mac");
                }
                for (std::vector<String>::iterator it = requiredKeys.begin(); it != requiredKeys.end(); ++it) {
                    if (!apiRequireParam(jsonObj, *it, request)) {
                        return;
                    }
                }

                AsyncJsonResponse *response = new AsyncJsonResponse();
                JsonObject root = response->getRoot();

                setCORSHeaders(request, response);

                if (type.equals("get_devices")) {
                    apiGetDevices(request, response, root);
                } else if (type.equals("get_wifi_networks")) {
                    apiGetWifiNetworks(request, response, root);
                } else if (type.equals("scan_wifi_networks")) {
                    apiScanWifiNetworks(request, response, root);
                } else if (type.equals("get_wifi_settings")) {
                    apiGetWifiSettings(request, response, root);
                } else if (type.equals("save_ap_name")) {
                    String name = jsonObj["name"].as<String>();
                    apiSaveApName(request, response, root, name);
                } else if (type.equals("connect_wifi")) {
                    String ssid = jsonObj["ssid"].as<String>();
                    String pass = jsonObj["pass"].as<String>();
                    apiWifiConnect(request, response, root, ssid, pass);
                } else if (type.equals("delete_device")) {
                    String id = jsonObj["id"].as<String>();
                    apiDeleteDevice(request, response, root, id);
                } else if (type.equals("save_device")) {
                    String id = jsonObj["id"].as<String>();
                    String name = jsonObj["name"].as<String>();
                    String mac = jsonObj["mac"].as<String>();
                    apiSaveDevice(request, response, root, id, name, mac);
                } else if (type.equals("save_now_mac")) {
                    String mac = jsonObj["mac"].as<String>();
                    apiSaveNowMac(request, response, root, mac);
                } else {
                    return request->send(400, "text/plain", "Unknown type");
                }

                response->setLength();
                request->send(response);
            });

        server->addHandler(handler);

        server->onNotFound(std::bind(&WebServer::notFound, this, std::placeholders::_1));

        server->begin();
    }
};
