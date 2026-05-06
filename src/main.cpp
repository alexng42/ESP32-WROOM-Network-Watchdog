#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#include <ESPAsyncWebServer.h> // Web Server library
#include <LittleFS.h>
#include "secrets.h"

// initialize the server on port 80
AsyncWebServer server(80);

String status = "Checking...";
int latency = 0;
unsigned long lastCheck = 0;

void setup() {
    Serial.begin(115200);

    // initialize LittleFS
    if(!LittleFS.begin(true)){
        Serial.println("LittleFS Mount Failed");
        return;
    }

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    Serial.println(WiFi.localIP());

    // Serve the HTML file from LittleFS
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html");
    });

    // Create a JSON API for the frontend to fetch
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"status\":\"" + status + "\",";
        json += "\"latency\":" + String(latency);
        json += "}";
        request->send(200, "application/json", json);
    });

    server.begin();
}

void loop() {
    if (millis() - lastCheck >= 10000) {
        lastCheck = millis();
        if (Ping.ping("8.8.8.8", 2)) {
            status = "ONLINE";
            latency = Ping.averageTime();
        } else {
            status = "OFFLINE";
            latency = 0;
        }
    }
}