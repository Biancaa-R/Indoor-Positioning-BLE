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
bool doConnect = false;

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
    BLEClient* pClient = BLEDevice::createClient();
    if (pClient->connect(myDevice)) {
      Serial.println("Connected!");
      BLERemoteService* pService = pClient->getService(SERVICE_UUID);
      if (pService) {
        BLERemoteCharacteristic* pChar = pService->getCharacteristic(CHARACTERISTIC_UUID);
        if (pChar) {
          String value = pChar->readValue().c_str();
          Serial.print("Received: ");
          Serial.println(value);
        } else Serial.println("Characteristic not found");
      } else Serial.println("Service not found");
      pClient->disconnect();
      Serial.println("Disconnected");
    } else Serial.println("Failed to connect");

    doConnect = false;
  }

  // Scan continuously
  pBLEScan->start(5, false);
  pBLEScan->clearResults();
  delay(2000);
}
