# Physical computing sensor wireless tutorial - Pollution monitor with ESP8266 and Adafruit IO

Fast WiFi and MQTT with deep-sleep to preserve battery capacity. Easily monitor PM2.5 and NC2.5, and transmit to Adafruit IO

### Setup

You can click the image to play the associated YouTube video

[![Alt text](Assets/3c%20result.jpg)](https://youtu.be/oAVPR7d2vUU/)

### Schematic

![](Assets/13f%20schematic.png)

### BOM

<pre>
€ 20,00 Adafruit Feather HUZZAH with ESP8266
€ 33,00 SENSIRION Particulate Matter Sensor SPS30
€  5,00 POLOLU U3V16F5 step-up converter
€ 19,00 10000mAh LiPo rechargeable battery w. JST-connector
€  1,00 Tactile button momentary switch
€  3,00 Half-size transparent breadboard
€  1,00 Breadboard mini modular black
€  1,00 Jumper cables
€  1,00 1 x 470Ω, 1 x 1kΩ and 2 x 10kΩ resistor
€ 84,00
</pre>  

### Useful links  

μc https://www.adafruit.com/product/3046  
ESP8266 library https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi  
Adafruit MQTT library https://github.com/adafruit/Adafruit_MQTT_Library  
Sensor https://sensirion.com/products/catalog/SPS30  
Sensor library https://github.com/Sensirion/arduino-i2c-sps30  
Sensor low-power operation application notes https://sensirion.com/media/documents/188A2C3C/6166F165/Sensirion_Particulate_Matter_AppNotes_SPS30_Low_Power_Operation_D1.pdf  
