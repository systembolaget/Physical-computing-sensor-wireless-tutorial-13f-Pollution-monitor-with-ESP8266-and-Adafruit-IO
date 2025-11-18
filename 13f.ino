// Tutorial 13f. Pollution monitor with ESP8266 and Adafruit IO

// Main parts: Adafruit Feather HUZZAH with ESP8266, SENSIRION SPS30
// particulate matter sensor, POLOLU U3V16F5 5V step-up converter,
// tactile button momentary switch, high capacity LiPo battery

// Libraries required for WiFi, Adafruit IO MQTT, and to interface
// with the sensor via I2C
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <SensirionI2cSps30.h>
#include <Wire.h>

// WiFi credentials, router channel, BSSID, and fixed IP. For a fast
// (= battery capacity saving) connection, set the router to use a
// fixed channel, and set a fixed IP address for your ESP8266. Use
// 13f_router_channel_MAC_address.ino to confirm the channel
// and to find your router's BSSID (= MAC address)
const char* ssid = "#";
const char* password = "#";
const uint8_t channel = 11;
const uint8_t bssid[] = { 0xB4, 0xFC, 0x7D, 0x9F, 0x5C, 0x51 };
IPAddress ip(192, 168, #, #);
IPAddress gateway(192, 168, #, #);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(1, 1, 1, 1);

// Adafruit IO server details and AIO credentials
#define AIO_SERVER "io.adafruit.com"
#define AIO_PORT 1883
#define AIO_USERNAME "#"
#define AIO_KEY "#"

// Instantiate a client and sensor object from the library
WiFiClient client;
SensirionI2cSps30 sensor;

// Instantiate an mqtt object with the server details and AIO credentials
// Instantiate two feed objects we can publish our sensor data to
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_PORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish MC_P2_5(&mqtt, AIO_USERNAME "/feeds/MCP2_5");
Adafruit_MQTT_Publish NC_P2_5(&mqtt, AIO_USERNAME "/feeds/NCP2_5");

// Variables that remain constant
const uint32_t timeSleep = 3e8; // 5 minutes, 6e8 = 10 minutes, 3e7 = 30 seconds for testing
const uint16_t timeConnectionWiFi = 2000; // Try to connect for two seconds max.
const uint16_t timeConnectionMQTT = 1000; // Try to connect for one second max.
const uint16_t timeStartup = 16000; // Recommended by the manufacturer
const uint16_t intervalSampling = 1000; // Recommended by the manufacturer
const uint8_t counterSamples = 30; // Recommended by the manufacturer

// Variables that can change
uint16_t mc1p0 = 0;
uint16_t mc2p5 = 0; // Mass of 2.5 um particles in micrograms per m3 of air
uint16_t mc4p0 = 0;
uint16_t mc10p0 = 0;
uint16_t nc0p5 = 0;
uint16_t nc1p0 = 0;
uint16_t nc2p5 = 0; // Number of 2.5 um particles per cm3 of air
uint16_t nc4p0 = 0;
uint16_t nc10p0 = 0;
uint16_t typicalParticleSize = 0;
uint16_t readingSPS30;
uint16_t readingsMc2p5;
uint16_t readingsNc2p5;
uint16_t averageMc2p5;
uint16_t averageNc2p5;
int8_t MQTTStatus; // 0 = connected, 3 = server is down

void setup()
  {
  // Use only for debugging and testing
  //Serial.begin(74880);

  // Initialise the I2C bus to communicate with the sensor
  Wire.begin();

  // Step 1 - We first initialise the sensor, then start its I2C
  // interface), and put it into the measuring mode; the UINT16
  // means that the measurements will be returned as integers
  sensor.begin(Wire, SPS30_I2C_ADDR_69);
  sensor.wakeUpSequence();
  sensor.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_UINT16);

  // Wait for fan and laser to stabilise before taking measurements
  // as recommended by the manufacturer
  delay(timeStartup);

  // Take 30 measurements, once per second, as recommended by the
  // manufacturer
  for (uint8_t i = 0; i < counterSamples; i++)
    {
    // We must take all measurements at once as mandated by the
    // manufacturer
    readingSPS30 = sensor.readMeasurementValuesUint16(mc1p0, mc2p5, mc4p0, mc10p0, nc0p5, nc1p0, nc2p5, nc4p0, nc10p0, typicalParticleSize);

    readingsMc2p5 += mc2p5;
    readingsNc2p5 += nc2p5;

    delay(intervalSampling);
    }

  // Calculate the average
  averageMc2p5 = readingsMc2p5 / counterSamples;
  averageNc2p5 = readingsNc2p5 / counterSamples;

  // We exit the measuring mode and put the sensor to sleep to save
  // battery capacity
  sensor.stopMeasurement();
  sensor.sleep();

  // Step 2 - Now that we have sensor data, it makes sense to spend
  // more battery capacity, so we make a WiFi connection

  // To connect to WiFi in under one second, we don't want
  // to waste time (battery capacity) by writing and reading our WiFi
  // settings to flash memory. We set the ESP8266 to be in client
  // (station) mode, supply the fixed IP address, and start WiFi with
  // our credentials, router channel and BSSID, as defined above
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet, dns);
  WiFi.begin(ssid, password, channel, bssid);

  // Timestamp that updates every time we try to connect
  uint32_t timeStartWiFi = millis();

  // We try connecting to the router for two seconds max.
  while (WiFi.status() != WL_CONNECTED && millis() - timeStartWiFi < timeConnectionWiFi)
    {
    delay(10);
    }

  // If we can't connect to the router, we put our set-up to deep
  // sleep, and try again in timeSleep minutes
  if (WiFi.status() != WL_CONNECTED)
    {
    // Only for testing and debugging
    //Serial.println("No WiFi");
    ESP.deepSleep(timeSleep);
    }

  // Step 3 - Now that we have a WiFi connection, it makes sense
  // to make the MQTT connection, and publish the data

  // Timestamp that updates every time we try to connect
  uint32_t timeStartMQTT = millis();

  // We try connecting to the MQTT server for one second max.
  while (((MQTTStatus = mqtt.connect()) != 0) && millis() - timeStartMQTT < timeConnectionMQTT)
    {
    delay(10);
    }

  // If we can't connect to the MQTT server, we put our set-up to
  // deep sleep, and try again in timeSleep minutes
  if ((MQTTStatus = mqtt.connect()) != 0)
    {
    // Only for testing and debugging
    //Serial.println("No MQTT");
    ESP.deepSleep(timeSleep);
    }

  // Now, the data is published to the Adafruit IO feeds
  MC_P2_5.publish(averageMc2p5);
  delay(100);
  NC_P2_5.publish(averageNc2p5);
  delay(100);

  // Properly disconnect from the MQTT server and WiFi router
  mqtt.disconnect();
  WiFi.disconnect();

  // Go to deep sleep to preserve battery capacity. The tactile
  // momentary switch allows us to wake up the set-up for ad-hoc
  // measurements; otherwise remove it, the resistor, and wiring
  ESP.deepSleep(timeSleep);
  }

void loop()
  {
  }
