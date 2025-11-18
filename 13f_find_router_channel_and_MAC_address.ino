#include <ESP8266WiFi.h>

const char* ssid = "Lagom";
const char* password = "63948100905083530879";

void setup()
  {
  Serial.begin(74880); // Use this baud rate for the ESP8266, serial only needed while debugging

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
    {
    delay(10);
    }

  Serial.printf("\nWiFi channel: %d\n", WiFi.channel());
  Serial.printf("WiFi BSSID: %s\n", WiFi.BSSIDstr().c_str());
  }

void loop()
  {
    ESP.deepSleep(0);
  }
