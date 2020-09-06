<img align="right" src="https://github.com/MecaHumArduino/ble-server/blob/master/docs/esp32.png?raw=true" style="max-width:100%;" height="350">

# ESP32 Coding Session 01
# Connect to WiFi and send data to AWS IoT


This coding session is the first video of a series where I will be exploring the ESP32 board following capabilities:

- Different WiFi modes
- Different Bluetooth abilities (Bluetooth Classic and BLE)
- Sending / Receiving data from AWS IoT

→ ESP32 dev board I used in this video: https://amzn.to/3ifzLSY

Youtube Video
========

[![ESP32 Coding Session 01](https://img.youtube.com/vi/G43hLHGPqQk/0.jpg)](https://www.youtube.com/watch?v=G43hLHGPqQk)

PART ONE
========

Description
--------------------------
In this video, I walk you through all the necessary steps to programme an ESP32 dev board to connect to WiFi, create a "Thing" on AWS IoT and secure it with a generated certificate, then use the certificate to connect the ESP32 board to the Thing. Then, we create a JSON document that we send to AWS IoT over an MQTT Topic every 2 seconds.

I'm using "VS Code" with "PlatformIO" plugin and standard Arduino libraries.

APPS
------
*   [VS Code](https://code.visualstudio.com/)
*   [platformIO](https://platformio.org/)

Libraries
--------------------
*   [WiFi](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi)
*   [ArduinoJson](https://arduinojson.org/v6/doc/)
*   [WiFiClientSecure](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiClientSecure)
*   [BLE](https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE)

PART TWO
========

Code Walk Through
--------------------

The code starts by including the required libraries. These libraries offer features to work with WiFi, Bluetooth, Json and more

```cpp
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include "WiFi.h"
#include "secrets.h"
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
```

The next line defines a few macros (Constant in other languages) we will use throughout the program:

We will use later to chose when to print something for debugging
```cpp
#define DEBUG
```
Names for the pub/sub MQTT topics
```cpp
#define AWS_IOT_SUBSCRIBE_TOPIC "thing/esp32/sub"
#define AWS_IOT_PUBLISH_TOPIC   "thing/esp32/pub"
```

Bluetooth service and characteristic IDs
```cpp
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
```

Then, create an instance `BLEServer` called `pServer`, an instance of `BLECharacteristic` called `pCharacteristic`, an instance of `WiFiClientSecure` called `net` and an instance of `MQTTClient` called `client`
```cpp
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);
```
### setup()
In the `setup()`, initialize a serial communication at a baud rate (speed) of 115200, but only if `debug` mode is defined (see macro above)

```cpp
#ifdef DEBUG
    Serial.begin(115200);
#endif
```
And call some self explanatory methods
```cpp
  connectToWIFI();
  connectToAWS();

  startBLEserver();
  startAdvertising();
```