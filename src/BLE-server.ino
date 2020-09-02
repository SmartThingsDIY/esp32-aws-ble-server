#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include "WiFi.h"
#include "secrets.h"
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#define DEBUG
// #define AWS

#ifdef AWS
#define AWS_IOT_SUBSCRIBE_TOPIC "thing/esp32/sub"
#define AWS_IOT_PUBLISH_TOPIC   "thing/esp32/pub"
#endif

// (promotion-type, promotion-discount, short-message)
#define BLE_ADVERTISING_STRING "p1,30,ladies night"
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

#ifdef AWS
WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);
#endif

void setup()
{
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  #ifdef AWS
  connectToWIFI();
  connectToAWS();
  #endif

  startBLEserver();
  startAdvertising();

  Serial.println("Listening for new devices");
}

void loop()
{
  // checkNewClients();

  #ifdef DEBUG
  Serial.println(".");
  #endif

  delay(2000);
}

class CharCallbacks: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    Serial.println("client wrote something...");

    if (value.length() > 0) {
      for (int i = 0; i < value.length(); i++) {
        Serial.print(value[i]);
      }

      Serial.println();
    }
  }

  void onRead(BLECharacteristic *pCharacteristic)
  {
    #ifdef DEBUG
    Serial.println("Message received by client");
    #endif
  }
};

class ServerCallbacks: public BLEServerCallbacks
{
  void onConnect(BLEServer* pServer)
  {
    deviceConnected = true;
    BLEDevice::startAdvertising();

    String deviceID = "uwieu3nms234";
    #ifdef DEBUG
    Serial.print("new device ");
    Serial.print(deviceID);
    Serial.println(" connected");
    #endif
    // publish on MQTT
  };

  void onDisconnect(BLEServer* pServer)
  {
    deviceConnected = false;

    String deviceID = "uwieu3nms234";
    #ifdef DEBUG
    Serial.print("device ");
    Serial.print(deviceID);
    Serial.println(" disconnected");
    #endif

    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising

    // publish on MQTT
  }
};

void startBLEserver()
{
  #ifdef DEBUG
  Serial.println("BLE server: Starting...");
  #endif

  BLEDevice::init("ESP32-BLE");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID,
                                          BLECharacteristic::PROPERTY_READ   |
                                          BLECharacteristic::PROPERTY_WRITE  |
                                          BLECharacteristic::PROPERTY_NOTIFY |
                                          BLECharacteristic::PROPERTY_INDICATE
                                       );

  pCharacteristic->setCallbacks(new CharCallbacks());

  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();
  Serial.println("BLE server: Started");
}

void startAdvertising()
{
  Serial.println("BLE advertising: starting...");

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // helps w/ iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE advertising: started");
}

#ifdef AWS
void connectToWIFI()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  #ifdef DEBUG
  Serial.println("Connecting to Wi-Fi");
  #endif

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}
#endif

#ifdef AWS
void connectToAWS()
{
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  // client.onMessage(messageHandler);

  #ifdef DEBUG
  Serial.println("AWS IoT: Connecting...");
  #endif

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected()) {
    #ifdef DEBUG
    Serial.println("AWS IoT: Timeout!");
    #endif
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  #ifdef DEBUG
  Serial.println("AWS IoT: Connected");
  #endif
}
#endif

#ifdef AWS
void messageHandler(String &topic, String &payload)
{
  Serial.println("incoming: " + topic + " - " + payload);
}
#endif

#ifdef AWS
void sendBackStats()
{
  StaticJsonDocument<200> doc;
  doc["time"]      = millis();
  doc["device_id"] = analogRead(0);

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
#endif

// void checkNewClients() {
//   if (deviceConnected) {
//     Serial.println("connected");

//     pCharacteristic->setValue(BLE_ADVERTISING_STRING);
//     pCharacteristic->notify();

//     // sendBackStats();
//     // client.loop();
//   }
//   if (!deviceConnected && oldDeviceConnected) {
//     delay(500); // give the bluetooth stack the chance to get things ready
//     pServer->startAdvertising(); // restart advertising
//     Serial.println("not connected");
//     Serial.println("start advertising");
//     oldDeviceConnected = deviceConnected;
//   }

//   // connecting
//   if (deviceConnected && !oldDeviceConnected) {
//     // do stuff here on connecting
//     oldDeviceConnected = deviceConnected;
//   }
// }