
#include <Arduino.h>
#include <buffer.hpp>
#include <com.hpp>

#if DEV_MODE == 1
#include <homie.hpp>
#include <webserver.hpp>
#include <websocket.hpp>

WebServer * webserver = new WebServer();
WebSocket * ws = new WebSocket(webserver);
GWHomie * homie = new GWHomie();
Com * comm = new Com(homie, ws);

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
#elif DEV_MODE == 2
    now->loop();
#endif
}

#if DEV_MODE != 1 && DEV_MODE != 2
#error DEV_MODE must be "1" for "web" or "2" for "now"
#endif
