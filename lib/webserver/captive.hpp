#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
        if (request->url().startsWith("/generate_204")) {
            return true;
        }

        return false;
    }

    void handleRequest(AsyncWebServerRequest *request) {
        request->redirect("http://" + WiFi.softAPIP().toString() + "/");
    }
};
