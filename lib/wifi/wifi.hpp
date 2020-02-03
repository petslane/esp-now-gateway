#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <config.hpp>

#define SCAN_PERIOD 5000

class WIFI {
  private:
    WiFiEventHandler wifiConnectHandler;
    WiFiEventHandler wifiDisconnectHandler;
    Ticker wifiReconnectTimer;

    DNSServer dnsServer;
    IPAddress apIP;
    IPAddress netMask;

    boolean testStarting = false;
    String testSSID;
    String testPass;

    boolean runningMDNS = false;
    boolean runningAP = false;

    void connectToWifi() {
        String ssid = config.wifi.ssid;
        String password = config.wifi.pass;

        testSSID = "";
        testPass = "";

        Serial.printf("[wifi] connecting to %s %s\n", ssid.c_str(), password.c_str());
        if (password.isEmpty()) {
            WiFi.begin(ssid);
        } else {
            WiFi.begin(ssid, password);
        }
    }

    void connectToTestWifi() {
        Serial.printf("[wifi] new connection to %s %s\n", testSSID.c_str(), testPass.c_str());
        testStarting = false;
        WiFi.begin(testSSID, testPass);
    }

    void onWifiConnect(const WiFiEventStationModeGotIP &event) {
        Serial.println("[wifi] wifi connected");
        std::for_each(onConnectCallbackVector.begin(),
                      onConnectCallbackVector.end(),
                      [](onCallback cb) { cb(); });

        if (!testSSID.isEmpty()) {
            Serial.println("[wifi] new WiFi connection success, saving new WiFi settings");
            config.wifi.ssid = testSSID;
            config.wifi.pass = testPass.isEmpty() ? "" : testPass;
            Config::saveConfig();
            testSSID = "";
            testPass = "";
        }

        if (!runningMDNS) {
            MDNS.begin("now-gw");
            MDNS.addService("http", "tcp", 80);
            runningMDNS = true;
        }
        Serial.printf("[wifi] IP: %s\n", event.ip.toString().c_str());
    }

    void onWifiDisconnect(const WiFiEventStationModeDisconnected &event) {
        Serial.println("[wifi] wifi disconnected");
        std::for_each(onDisconnectCallbackVector.begin(),
                      onDisconnectCallbackVector.end(),
                      [](onCallback cb) { cb(); });

        if (!testStarting) { // skip reconnect if new connection test is started
            wifiReconnectTimer.once(5, std::bind(&WIFI::connectToWifi, this));
        }

        if (!testSSID.isEmpty() && !testStarting) {
            testSSID = "";
            testPass = "";
            Serial.println("[wifi] new WiFi connection failed");
        }

        if (runningMDNS) {
            MDNS.end();
            runningMDNS = false;
        }

        if (!runningAP) {
            Serial.println("[wifi] AP started");
            WiFi.softAP(config.ap.name);
            WiFi.mode(WIFI_AP_STA);
            runningAP = true;
        }
    }

    typedef std::function<void(void)> onCallback;
    std::vector<onCallback> onConnectCallbackVector;
    std::vector<onCallback> onDisconnectCallbackVector;

  public:
    WIFI() {
        wifiConnectHandler = WiFi.onStationModeGotIP(
            std::bind(&WIFI::onWifiConnect, this, std::placeholders::_1));
        wifiDisconnectHandler = WiFi.onStationModeDisconnected(
            std::bind(&WIFI::onWifiDisconnect, this, std::placeholders::_1));

        testSSID = "";
        testPass = "";

        apIP = WiFi.softAPIP();

        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(53, "*", apIP);
    }

    void testWifiConnection(String SSID, String Pass) {
        // disconnect before starting new test connect
        WiFi.disconnect(false);

        testStarting = true;
        testSSID = SSID;
        testPass = Pass;

        wifiReconnectTimer.once(1, std::bind(&WIFI::connectToTestWifi, this));
    }

    void setup() {
        WiFi.mode(WIFI_STA);
        connectToWifi();
    }

    void loop() {
        dnsServer.processNextRequest();
        if (runningMDNS) {
            MDNS.update();
        }
        if (runningAP && WiFi.isConnected() &&
            WiFi.softAPgetStationNum() == 0) {
            Serial.println("[wifi] AP stopped");
            WiFi.mode(WIFI_STA);
            WiFi.softAPdisconnect(true);
            runningAP = false;
        }
    }
};
