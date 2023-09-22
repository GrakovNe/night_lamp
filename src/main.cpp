#include <Arduino.h>
#include <ESP8266mDNS.h>

#define SWITCH_PIN D5
#define BUTTON_PIN D2

#include "lib/WiFiManager.h"
#include "ui.h"

#define DEVICE_NAME "night_lamp"

ESP8266WebServer httpServer(80);
WiFiManager wm;

bool disabled;

String current_state_to_json() {
    return "{"
           "\"enabled\": " +
           String(!disabled) +
           "}";
}

void restServerRouting() {
    httpServer.on("/state", HTTP_GET, []() {
        httpServer.sendHeader("Access-Control-Allow-Origin", "*");
        httpServer.send(200, F("application/json"), current_state_to_json());
    });

    httpServer.on("/enable", HTTP_POST, []() {
        httpServer.sendHeader("Access-Control-Allow-Origin", "*");
        disabled = false;
        httpServer.send(200, F("application/json"), current_state_to_json());
    });

    httpServer.on("/disable", HTTP_POST, []() {
        httpServer.sendHeader("Access-Control-Allow-Origin", "*");
        disabled = true;
        httpServer.send(200, F("application/json"), current_state_to_json());
    });

    httpServer.onNotFound([]() {
        httpServer.sendHeader("Access-Control-Allow-Origin", "*");
        httpServer.send(200, F("text/html; charset=utf-8"), mainPage);
    });

}

void configure_wlan() {
    wm.setTitle(DEVICE_NAME);

    WiFi.hostname(DEVICE_NAME);
    wm.setHostname(DEVICE_NAME);
    wm.setHttpPort(8080);

    WiFi.mode(WIFI_STA);
    wm.setCaptivePortalEnable(false);
    wm.setConfigPortalBlocking(false);

    if (wm.autoConnect(DEVICE_NAME)) {
        wm.startConfigPortal();
    }

    restServerRouting();
    httpServer.begin();

    MDNS.begin(DEVICE_NAME, WiFi.localIP());
}

void setup() {
    configure_wlan();

    pinMode(SWITCH_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
    wm.process();
    httpServer.handleClient();
    MDNS.update();

    if (!digitalRead(BUTTON_PIN)) {
        disabled = !disabled;
        delay(500);
    }

    digitalWrite(SWITCH_PIN, disabled);
}