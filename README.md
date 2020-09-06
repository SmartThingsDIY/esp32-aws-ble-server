<img align="right" src="https://github.com/MecaHumArduino/ble-server/blob/master/docs/esp32.png?raw=true" style="max-width:100%;" height="350">

# ESP32 Coding Sessions
## Connect to WiFi and send data to AWS IoT
## Create a BLE server and interact with devices through Bluetooth


This coding session is the first video of a series where I will be exploring the ESP32 board following capabilities:

- Different WiFi modes
- Different Bluetooth abilities (Bluetooth Classic and BLE)
- Sending / Receiving data from AWS IoT

→ ESP32 dev board I used in this tutorial: https://amzn.to/3ifzLSY

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
Followed by printing another debug message. At this point, the ESP32 has started the Bluetooth server and is looking for client devices
```cpp
#ifdef DEBUG
    Serial.println("Listening for new devices");
#endif
```

### connectToWIFI()
This function starts by setting the WiFi mode to `STA`. The Station (STA) mode is used to get the ESP32 connected to a WiFi network established by an access point, so basically it's setting up to connect to your home network.
```cpp
WiFi.mode(WIFI_STA);
```
Then, initializes the WiFi library's network settings with the credentials defined in the [secrets.h](https://github.com/MecaHumArduino/esp32-aws-ble-server/blob/master/include/secrets_copy.h) file and provides the current status.
```cpp
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
```

Prints a debug message indicating the operation has started
```cpp
#ifdef DEBUG
    Serial.println("Connecting to Wi-Fi");
#endif
```
And waits for the connection to be established while printing a "." every half a second
```cpp
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}
```

### connectToAWS()
After the ESP32 is connected to the WiFi network, it's time to connect to AWS IoT using the certificate generated earlier ([check the video for the steps](https://www.youtube.com/watch?v=G43hLHGPqQk))
```cpp
// Configure WiFiClientSecure to use the AWS IoT device credentials
net.setCACert(AWS_CERT_CA);
net.setCertificate(AWS_CERT_CRT);
net.setPrivateKey(AWS_CERT_PRIVATE);
```

Connect to the MQTT broker on the AWS endpoint and the certifications defined earlier, it uses the default 8883 port
```cpp
client.begin(AWS_IOT_ENDPOINT, 8883, net);
```

This line defines a way to handel incoming messages from AWS IoT through the MQTT Topic we subscribed to. In this instance, every time a message comes through, we call `messageHandler`
```cpp
client.onMessage(messageHandler);
```

Print a debug message and try to establish the connection to our Thing using its `THINGNAME` while printing a "." every 100ms
```cpp
#ifdef DEBUG
    Serial.println("AWS IoT: Connecting...");
#endif

while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
}
```

Prints a debug message and exits if connection can not be established
```cpp
if (!client.connected()) {
    Serial.println("AWS IoT: Timeout!");
    return;
}
```

Otherwise, if connection is successful, then subscribe to the topic defined above and prints another success debug message
```cpp
client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

#ifdef DEBUG
    Serial.println("AWS IoT: Connected");
#endif
```

### startBLEserver()
Initialize a new BLE (BlueTooth Low Energy) server and call it `ESP32-BLE`. This server will be discoverable by smartphones or any other compatible device
```cpp
BLEDevice::init("ESP32-BLE");
pServer = BLEDevice::createServer();
```

Set a function that will handle all callbacks received by the server
```cpp
pServer->setCallbacks(new ServerCallbacks());
```

Create a service and attach it to the server while giving it ability to read, write and notify any connected device.
```cpp
BLEService *pService = pServer->createService(SERVICE_UUID);
pCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID,
                                          BLECharacteristic::PROPERTY_READ   |
                                          BLECharacteristic::PROPERTY_WRITE  |
                                          BLECharacteristic::PROPERTY_NOTIFY
                                       );
```

Set a function that will handle all callbacks that interact will the characteristic
```cpp
pCharacteristic->setCallbacks(new CharCallbacks());
```

And finally start the server
```cpp
pService->start();
```

### startAdvertising()
Once the BLE server has started, it's time to start advertising it, which basically means making it public and waiting for connections.
```cpp
BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
pAdvertising->addServiceUUID(SERVICE_UUID);
pAdvertising->setScanResponse(true);
pAdvertising->setMinPreferred(0x06);  // helps w/ iPhone connections issue
pAdvertising->setMinPreferred(0x12);
BLEDevice::startAdvertising();
```