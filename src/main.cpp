
#include <Arduino.h>
#include <buffer.hpp>
#include <com.hpp>

#if DEV_MODE == 1
#include <homie.hpp>
#include <webserver.hpp>
#include <websocket.hpp>

#ifdef ENABLE_OLED_SHIELD
#include <screen.hpp>
#endif

WebServer * webserver = new WebServer();
WebSocket * ws = new WebSocket(webserver);
GWHomie * homie = new GWHomie();
Com * comm = new Com(homie, ws);
#ifdef ENABLE_OLED_SHIELD
Screen * screen = new Screen();
#endif

#elif DEV_MODE == 2
#include <now.hpp>

Com * comm = new Com();
Now * now = new Now(comm);
#endif

void setup() {
    Serial.begin(115200);
    Buffer::setup();
    comm->setup();
#if DEV_MODE == 1
    homie->setup();
    webserver->setup();
    ws->setup();
#ifdef ENABLE_OLED_SHIELD
    screen->setup();
#endif
#elif DEV_MODE == 2
    now->setup();
#endif
    Serial.println("Setup");
}


void loop() {
    comm->loop();
#if DEV_MODE == 1
    homie->loop();
    ws->loop();
#ifdef ENABLE_OLED_SHIELD
    screen->loop();
#endif
#elif DEV_MODE == 2
    now->loop();
#endif
}

#if DEV_MODE != 1 && DEV_MODE != 2
#error DEV_MODE must be "1" for "web" or "2" for "now"
#endif
