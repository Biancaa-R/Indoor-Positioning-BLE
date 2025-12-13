#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// --- UUIDs ---
// Generate your own from: https://www.uuidgenerator.net/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_1 "beb5483e-36e1-4688-b7f5-ea07361b26a9"


BLECharacteristic *pCharacteristic;
BLECharacteristic *pCharacteristic_1;

void setupBLE() {
  Serial.println("Initializing BLE...");
  BLEDevice::init("MyESP32");

  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE
                    );

    pCharacteristic_1 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_1,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE 
                    );

  pCharacteristic->setValue("Hello world !");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  //helping the ble to connect to phone
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE Advertising started!");
}

void setup() {
  Serial.begin(115200);
  Serial.println("System Booting...");

  //attachInterrupt(digitalPinToInterrupt(PIR_PIN), detectsMovement, RISING);

  setupBLE();

  Serial.println("System Ready!");
}

void loop() {
  //second one is for recieving the data
  String value=pCharacteristic_1->getValue();
  Serial.println(value.c_str());
  pCharacteristic->setValue(value.c_str());
  pCharacteristic->notify();  // Send BLE notification

  Serial.println("-----------------------------");
  delay(5000);
}
