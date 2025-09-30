#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 5; // in seconds
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String name = advertisedDevice.getName().c_str();
    if (name == "") name = "Unknown";

    int rssi = advertisedDevice.getRSSI();
    Serial.printf("Device: %s, RSSI: %d ", advertisedDevice.getAddress().toString().c_str(), rssi);

    // Draw a simple ASCII bar graph
    int barLength = map(rssi, -100, 0, 0, 50); // RSSI -100 (weak) to 0 (strong) mapped to 0-50 chars
    if (barLength < 0) barLength = 0;
    for (int i = 0; i < barLength; i++) Serial.print("|");
    Serial.println();
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); // create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  BLEScanResults* foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices->getCount());
  Serial.println("Scan done!\n");
  pBLEScan->clearResults(); // delete results from BLEScan buffer to release memory
  delay(2000);
}
