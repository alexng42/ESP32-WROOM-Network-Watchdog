#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#include "secrets.h"

void setup() {
  Serial.begin(115200);
  
  // 1. Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  Serial.print("Pinging Google (8.8.8.8)... ");
  
  // 2. The Watchdog Logic
  bool success = Ping.ping("8.8.8.8", 3); // Pings 3 times
  
  if (success) {
    int avgTime = Ping.averageTime();
    Serial.printf("SUCCESS Average response time: %d ms\n", avgTime);
  } else {
    Serial.println("FAIL Host unreachable.");
  }

  // Wait 10 seconds before checking again
  delay(10000); 
}