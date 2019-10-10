#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "stats.hpp"

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

class Screen {
private:
    unsigned long lastUpdate;
    uint8 loader;
    Stats * stats;
public:
    Screen(Stats * stats) {
        lastUpdate = 0;
        loader = 0;

        this->stats = stats;
    }
    void setup() {
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

        display.clearDisplay();
        display.display();
    }

    void loop() {
        if (millis() - lastUpdate > 300) {
            lastUpdate = millis();

            update();
        }
    }

    void update() {
        loader = (loader + 1) % 4;
        char spinner[] = "-\\|/";

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);

        // WiFi
        station_status_t status = wifi_station_get_connect_status();
        switch (status) {
            case STATION_GOT_IP:
                display.println("WiFi OK");
                break;
            case STATION_NO_AP_FOUND:
                display.println("WiFi No AP");
                break;
            case STATION_CONNECT_FAIL:
                display.println("WiFi Fail");
                break;
            case STATION_WRONG_PASSWORD:
                display.println("Wrong WiFi password");
                break;
            case STATION_IDLE:
            default:
                display.print("WiFi ");
                display.println(spinner[loader]);

        }

        // MQTT
        if (Homie.getMqttClient().connected()) {
            display.println("MQTT OK");
        } else if (status == STATION_GOT_IP) {
            display.print("MQTT ");
            display.println(spinner[loader]);
        } else {
            display.println("MQTT");
        }

        // Messages count
        display.println("Msg (i/o):");
        display.print(stats->getNowMessagesReceived());
        display.print("/");
        display.println(stats->getNowSentMessagesSuccessful());

        int buffer1FreeRelative = stats->getIncomingBufferSize() ? 100 * stats->getIncomingBufferFree() / stats->getIncomingBufferSize() : 0;
        int buffer2FreeRelative = stats->getMessageBufferSize() ? 100 * stats->getMessageBufferFree() / stats->getMessageBufferSize() : 0;
        display.drawLine(0, 46, 63 * buffer1FreeRelative / 100, 46, WHITE);
        display.drawLine(0, 47, 63 * buffer2FreeRelative / 100, 47, WHITE);

        display.display();
    }
};
