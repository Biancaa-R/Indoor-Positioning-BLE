#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>

#define TARGET_DEVICE_MAC "26:17:9D:4C:DF:C5"
#define SERVICE_UUID        "0000180d-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID "00002a37-0000-1000-8000-00805f9b34fb"

BLEScan* pBLEScan;
int scanTime = 5;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    Serial.print("Found device: ");
    Serial.println(advertisedDevice.getAddress().toString().c_str());
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
  Serial.println("Scanning...");
  BLEScanResults* foundDevices = pBLEScan->start(scanTime, false);  // **pointer now**
  Serial.print("Devices found: ");
  Serial.println(foundDevices->getCount());

  for (int i = 0; i < foundDevices->getCount(); i++) {
    BLEAdvertisedDevice device = foundDevices->getDevice(i);
    if (device.getAddress().toString() == TARGET_DEVICE_MAC) {
      Serial.println("Connecting...");
      BLEClient* pClient = BLEDevice::createClient();
      if (pClient->connect(&device)) {
        Serial.println("Connected!");

        BLERemoteService* pService = pClient->getService(SERVICE_UUID);
        if (pService) {
          BLERemoteCharacteristic* pChar = pService->getCharacteristic(CHARACTERISTIC_UUID);
          if (pChar) {
            String value = pChar->readValue();  // **Arduino String**
            Serial.print("Characteristic value: ");
            Serial.println(value);
          } else Serial.println("Characteristic not found");
        } else Serial.println("Service not found");

        pClient->disconnect();
        Serial.println("Disconnected");
      } else Serial.println("Failed to connect");
    }
  }

  pBLEScan->clearResults();  
  delay(5000);
}
