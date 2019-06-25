
#include <Arduino.h>
#include <buffer.hpp>
#include <com.hpp>

#if DEV_MODE == 1
#include <homie.hpp>
#include <webserver.hpp>
#include <websocket.hpp>
#elif DEV_MODE == 2
#include <now.hpp>
#endif

void setup() {
    Serial.begin(115200);
    Buffer::setup();
    com::setup();
#if DEV_MODE == 1
    com::mqttSend = homie::send;
    homie::setup();
    webserver::setup();
    ws::setup();
#elif DEV_MODE == 2
    now::serialSend = com::send;
    now::setup();
#endif
    Serial.println("Setup");
}


void loop() {
    com::loop();
#if DEV_MODE == 1
    homie::loop();
    ws::loop();
#elif DEV_MODE == 2
    now::loop();
#endif
}

#if DEV_MODE != 1 && DEV_MODE != 2
#error DEV_MODE must be "1" for "web" or "2" for "now"
#endif
