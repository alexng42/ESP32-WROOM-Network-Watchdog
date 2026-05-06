#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#include <ESPAsyncWebServer.h> // Web Server library
#include "secrets.h"

// initialize the server on port 80
AsyncWebServer server(80);

// global variables to store the state
String lastStatus = "Checking...";
int lastLatency = 0;
unsigned long lastCheckTime = 0;
const long interval = 10000; // check every 10 seconds

// Backend Logic
void checkNetwork()
{
  Serial.print("Watchdog checking Google... ");
  if (Ping.ping("8.8.8.8", 2))
  {
    lastStatus = "ONLINE";
    lastLatency = Ping.averageTime();
    Serial.println("SUCCESS");
  }
  else
  {
    lastStatus = "OFFLINE";
    lastLatency = 0;
    Serial.println("FAIL");
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // waiting to connect to wifi
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected. IP: " + WiFi.localIP().toString());

  // API + Frontend
  /*
  server.on(URL path (/ is root IP), HTTP method (HTTP_GET/POST/DELETE/etc), callback function (what to do when route is hit))
  */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        // basic frontend
        String html = "<html><body style='font-family:sans-serif; text-align:center;'>";
        html += "<h1>ESP32 Network Watchdog</h1>";
        html += "<p>Status: <strong>" + lastStatus + "</strong></p>";
        html += "<p>Latency: " + String(lastLatency) + "ms</p>";
        html += "<button onclick='location.reload()'>Refresh</button>";
        html += "</body></html>";
        
        request->send(200, "text/html", html); });

  // start server
  server.begin();

  // initial check
  checkNetwork();
}

void loop()
{
  // non-blocking timer logic
  unsigned long currentMillis = millis();
  if (currentMillis - lastCheckTime >= interval)
  {
    lastCheckTime = currentMillis;
    checkNetwork();
  }
}