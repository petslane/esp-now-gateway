#ifndef WEBSERVER_INIT_H
#define WEBSERVER_INIT_H

#define XSTR(s) STR(s)
#define STR(s) #s

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "utils.hpp"

namespace webserver {

    AsyncWebServer server(80);

    void notFound(AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    }

    bool authenticateRequest(AsyncWebServerRequest *request) {
#ifndef AUTH_USERNAME
        return true;
#endif
        if (!request->authenticate((const char *) XSTR(AUTH_USERNAME), (const char *) XSTR(AUTH_PASSWORD))) {
            Serial.println("[web] Invalid username/password");
            request->requestAuthentication();
            return false;
        }

        return true;
    }

    bool writeDevicesJson(String data) {
        File f = SPIFFS.open("/devices.json", "w");
        if (!f) {
            return false;
        }

        Serial.printf("Writing devices.json\n%s\nEOF\n", data.c_str());
        f.write(data.c_str(), data.length());
        f.close();

        return true;
    }

    bool readDevicesJson(String * out) {
        if (!SPIFFS.exists("/devices.json")) {
            * out = "devices.json file missing";
            return false;
        }

        File f = SPIFFS.open("/devices.json", "r");
        if (!f) {
            * out = "unable to open devices.json file";
            return false;
        }

        DynamicJsonBuffer jsonBuffer;
        JsonObject &fileRoot = jsonBuffer.parseObject(f);
        f.close();

        if (!fileRoot.success()) {
            * out = "Invalid devices.json content";
            return false;
        }

        String tmp;
        fileRoot.printTo(tmp);
        *out = tmp;

        return true;
    }

    void apiGetDevices(AsyncWebServerRequest *request) {
        AsyncJsonResponse * response = new AsyncJsonResponse();
        JsonObject& root = response->getRoot();

        StaticJsonBuffer<2048> jsonBuffer;
        String raw;
        if (!readDevicesJson(& raw)) {
            root["status"] = false;
            root["error"] = raw;
            response->setLength();
            request->send(response);
            return;
        }

        JsonObject &fileRoot = jsonBuffer.parseObject(raw);

        root["status"] = true;
        root.createNestedObject("data");
        utils::mergeJson(root["data"], fileRoot);
        response->setLength();
        request->send(response);
    }

    void apiDeleteDevice(AsyncWebServerRequest *request, String id) {
        AsyncJsonResponse * response = new AsyncJsonResponse();
        JsonObject& root = response->getRoot();

        StaticJsonBuffer<2048> jsonBuffer;
        String raw;
        if (!readDevicesJson(& raw)) {
            root["status"] = false;
            root["error"] = raw;
            response->setLength();
            request->send(response);
            return;
        }

        JsonObject &fileRoot = jsonBuffer.parseObject(raw);

        if (!fileRoot.containsKey(id)) {
            root["status"] = false;
            root["error"] = "Device " + id + " not found";
            response->setLength();
            request->send(response);
            return;
        }

        fileRoot.remove(id);

        String out;
        fileRoot.printTo(out);
        if (!writeDevicesJson(out)) {
            root["status"] = false;
            root["error"] = "Unable to write a file";
            response->setLength();
            request->send(response);
            return;
        }

        root["status"] = true;
        root.createNestedObject("data");
        utils::mergeJson(root["data"], fileRoot);
        response->setLength();
        request->send(response);
    }

    void apiSaveDevice(AsyncWebServerRequest *request, String id, String name, String mac) {
        AsyncJsonResponse * response = new AsyncJsonResponse();
        JsonObject& root = response->getRoot();

        StaticJsonBuffer<2048> jsonBuffer;
        String raw;
        if (!readDevicesJson(& raw)) {
            root["status"] = false;
            root["error"] = raw;
            response->setLength();
            request->send(response);
            return;
        }

        JsonObject &fileRoot = jsonBuffer.parseObject(raw);

        if (id.length()) {
            if (!fileRoot.containsKey(id)) {
                root["status"] = false;
                root["error"] = "Device " + id + " not found";
                response->setLength();
                request->send(response);
                return;
            }
            fileRoot.remove(id);
        }

        if (!mac.length()) {
            root["status"] = false;
            root["error"] = "Device mac address not provided";
            response->setLength();
            request->send(response);
            return;
        }
        if (!name.length()) {
            root["status"] = false;
            root["error"] = "Device name not provided";
            response->setLength();
            request->send(response);
            return;
        }
        if (!id.length() && fileRoot.containsKey(mac)) {
            root["status"] = false;
            root["error"] = "Device with provided mac address already exists";
            response->setLength();
            request->send(response);
            return;
        }

        fileRoot.createNestedObject(mac);
        fileRoot[mac]["name"] = name;

        String out;
        fileRoot.printTo(out);
        if (!writeDevicesJson(out)) {
            root["status"] = false;
            root["error"] = "Unable to write a file";
            response->setLength();
            request->send(response);
            return;
        }

        root["status"] = true;
        root.createNestedObject("data");
        utils::mergeJson(root["data"], fileRoot);
        response->setLength();
        request->send(response);
    }

    bool apiRequireParam(JsonObject& jsonObj, String param, AsyncWebServerRequest *request) {
        if (!jsonObj.containsKey(param)) {
            request->send(400, "text/plain", "Missing parameter " + param);
            return false;
        }

        return true;
    }

    void setup() {
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            if (!authenticateRequest(request)) {
                return;
            }

            AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/www/index.html.gz", "text/html");
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        });
        server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            if (!authenticateRequest(request)) {
                return;
            }

            AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/www/app.js.gz", "application/javascript");
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        });

        server.on("/api", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
            AsyncWebServerResponse *response = request->beginResponse(200);
            response->addHeader("Access-Control-Allow-Headers","Content-Type");
            request->send(response);
        });

        AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/api", [](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!authenticateRequest(request)) {
                return;
            }

            JsonObject& jsonObj = json.as<JsonObject>();

            if (!apiRequireParam(jsonObj, "type", request)) {
                return;
            }

            String type = jsonObj.get<String>("type");


            std::vector<String> requiredKeys;
            if (type.equals("get_devices")) {

            } else if (type.equals("delete_device")) {
                requiredKeys.emplace_back("id");
            } else if (type.equals("save_device")) {
                requiredKeys.emplace_back("name");
                requiredKeys.emplace_back("mac");
            }
            for (std::vector<String>::iterator it = requiredKeys.begin() ; it != requiredKeys.end(); ++it) {
                if (!apiRequireParam(jsonObj, *it, request)) {
                    return;
                }
            }

            if (type.equals("get_devices")) {
                return apiGetDevices(request);
            } else if (type.equals("delete_device")) {
                String id = jsonObj.get<String>("id");
                return apiDeleteDevice(request, id);
            } else if (type.equals("save_device")) {
                String id = jsonObj.get<String>("id");
                String name = jsonObj.get<String>("name");
                String mac = jsonObj.get<String>("mac");
                return apiSaveDevice(request, id, name, mac);
            }

            return request->send(400, "text/plain", "Unknown type");
        });

        server.addHandler(handler);

        server.onNotFound(notFound);
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

        server.begin();
    }

}

#endif // WEBSERVER_INIT_H
