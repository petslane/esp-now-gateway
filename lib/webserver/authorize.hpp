#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <config.hpp>

class AuthorizeRequestHandler : public AsyncWebHandler {
  public:
    AuthorizeRequestHandler() {
    }

    virtual ~AuthorizeRequestHandler() {
    }

    bool canHandle(AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            // skip OPTIONS requests
            return false;
        }

        if (!config.web.auth.username->length()) {
            // auth not configured
            return false;
        }

        if (request->authenticate(config.web.auth.username->c_str(), config.web.auth.password->c_str())) {
            // username/password OK,
            return false;
        }

        // username/password incorrect, request authentication
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
        Serial.println("[web] Invalid username/password");
        request->requestAuthentication();
    }
};
