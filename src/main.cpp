#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#include <ESPAsyncWebServer.h> // Web Server library
#include <LittleFS.h>
#include "secrets.h"
#include <time.h>

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -28800;   // PST
const int daylightOffset_sec = 3600; // 1 hour for PDT

// initialize the server on port 80
AsyncWebServer server(80);

String status = "Checking...";
int latency = 0;
unsigned long lastCheck = 0;

void logEvent(String message)
{
  File file = LittleFS.open("/log.txt", FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open log file");
    return;
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    file.println("Time Error: " + message);
  }
  else
  {
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    file.printf("[%s] %s\n", timeStringBuff, message.c_str());
  }
  file.close();
}

void setup()
{
  Serial.begin(115200);

  // initialize LittleFS
  if (!LittleFS.begin(true))
  {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println(WiFi.localIP());
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Serve the HTML file from LittleFS
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });

  // Create a JSON API for the frontend to fetch
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        String json = "{";
        json += "\"status\":\"" + status + "\",";
        json += "\"latency\":" + String(latency);
        json += "}";
        request->send(200, "application/json", json); });

  server.on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/log.txt", "text/plain"); });

  server.on("/api/clear", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        LittleFS.remove("/log.txt");
        request->send(200, "text/plain", "Logs cleared"); });

  server.begin();
}

String previousStatus = "";

void loop()
{
  if (millis() - lastCheck >= 10000)
  {
    lastCheck = millis();
    bool currentPing = Ping.ping("8.8.8.8", 2);
    status = currentPing ? "ONLINE" : "OFFLINE";
    latency = currentPing ? Ping.averageTime() : 0;

    // log if the status changed
    if (status != previousStatus)
    {
      logEvent("Status changed to " + status);
      previousStatus = status;
    }
  }
}