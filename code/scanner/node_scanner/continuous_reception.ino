#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>

#define TARGET_DEVICE_NAME "MyESP32"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEScan* pBLEScan;
BLEAdvertisedDevice* myDevice;
BLEClient* pClient = nullptr;
BLERemoteCharacteristic* pRemoteChar = nullptr;
bool doConnect = false;
bool connected = false;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    if (advertisedDevice.getName() == TARGET_DEVICE_NAME) {
      Serial.println("Found broadcaster!");
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
    }
  }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("");
  
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  if (doConnect && myDevice != nullptr) {
    Serial.println("Connecting...");
    pClient = BLEDevice::createClient();
    
    if (pClient->connect(myDevice)) {
      Serial.println("Connected!");
      BLERemoteService* pService = pClient->getService(SERVICE_UUID);
      if (pService) {
        pRemoteChar = pService->getCharacteristic(CHARACTERISTIC_UUID);
        if (pRemoteChar) {
          Serial.println("Ready to read dynamic values!");
          connected = true;
        } else Serial.println("Characteristic not found");
      } else Serial.println("Service not found");
    } else Serial.println("Failed to connect");

    doConnect = false;  // Only try to connect once
  }

  // If connected, read periodically
  if (connected && pRemoteChar != nullptr) {
    String value = pRemoteChar->readValue().c_str();
    Serial.print("Dynamic value: ");
    Serial.println(value);
  } else {
    // Scan again if not connected
    pBLEScan->start(5, false);
    pBLEScan->clearResults();
  }

  delay(2000);
}
