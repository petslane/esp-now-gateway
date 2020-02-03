
#include <Arduino.h>
#include <buffer.hpp>
#include <com.hpp>
#include <stats.hpp>
Stats * stats = new Stats();

#if DEV_MODE == 1
#include <webserver.hpp>
#include <websocket.hpp>
#include <config.hpp>
#include <mqtt.hpp>
#include <wifi.hpp>

#ifdef ENABLE_OLED_SHIELD
#include <screen.hpp>
#endif

MQTT * mqtt = new MQTT();
WIFI * wifi = new WIFI();
WebServer * webserver = new WebServer(wifi);
WebSocket * ws = new WebSocket(webserver, stats);
Com * comm = new Com(stats, ws, mqtt);
#ifdef ENABLE_OLED_SHIELD
Screen * screen = new Screen(stats, mqtt);
#endif

#elif DEV_MODE == 2
#include <now.hpp>

Com * comm = new Com(stats);
Now * now = new Now(comm, stats);
#endif

void setup() {
    Serial.begin(115200);
    Buffer::setup();
    comm->setup();
#if DEV_MODE == 1
    Config::load();
    mqtt->setup();
    wifi->setup();
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
    stats->loop();
#if DEV_MODE == 1
    ws->loop();
    wifi->loop();
    mqtt->loop();
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
