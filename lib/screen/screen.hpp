
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET D3  // GPIO0
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
public:
    Screen() {
        lastUpdate = 0;
        loader = 0;
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
        display.println("0000/9999");

        display.display();
    }
};
