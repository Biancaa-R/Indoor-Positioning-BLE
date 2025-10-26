#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// --- UUIDs ---
// Generate your own from: https://www.uuidgenerator.net/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// --- Pins ---
#define LED2      15
#define LED3      2
#define BUTTON    0
#define VIB_PIN   36
#define TRIG_PIN  4
#define ECHO_PIN  16
#define PIR_PIN   27
#define LED_PIR   26

// --- Thresholds / timers ---
const int vibThreshold = 300;
const unsigned long ultrasonicTimeout = 30000UL;
const int pirHoldSeconds = 10;

// --- System state ---
int lastButtonState = HIGH;
unsigned long lastButtonMillis = 0;
bool systemOn = false;

// --- PIR state ---
volatile bool pirTriggered = false;
bool motionDetected = false;
unsigned long lastPirTrigger = 0;

// --- BLE globals ---
BLECharacteristic *pCharacteristic;

// --- PIR interrupt ---
void IRAM_ATTR detectsMovement() {
  pirTriggered = true;
  lastPirTrigger = millis();
}

// --- Ultrasonic ---
float measureDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ultrasonicTimeout);
  if (duration == 0) return -1.0;
  return (duration * 0.0343f) / 2.0f;
}

// --- PIR handler ---
void handlePIR() {
  unsigned long now = millis();

  if (pirTriggered) {
    if (!motionDetected) Serial.println("MOTION DETECTED!!!");
    digitalWrite(LED_PIR, HIGH);
    motionDetected = true;
    pirTriggered = false;
  }

  if (motionDetected && (now - lastPirTrigger > pirHoldSeconds * 1000UL)) {
    Serial.println("Motion stopped...");
    digitalWrite(LED_PIR, LOW);
    motionDetected = false;
  }
}

// --- BLE setup ---
void setupBLE() {
  Serial.println("Initializing BLE...");
  BLEDevice::init("MyESP32");

  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->setValue("Waiting for sensor data...");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE Advertising started!");
}

void setup() {
  Serial.begin(115200);
  Serial.println("System Booting...");

  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  pinMode(VIB_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIR, OUTPUT);
  digitalWrite(LED_PIR, LOW);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), detectsMovement, RISING);

  setupBLE();

  Serial.println("System Ready!");
}

void loop() {
  unsigned long now = millis();

  // --- Button toggle ---
  int buttonState = digitalRead(BUTTON);
  if (buttonState == LOW && lastButtonState == HIGH && (now - lastButtonMillis > 200)) {
    systemOn = !systemOn;
    digitalWrite(LED2, systemOn ? HIGH : LOW);
    digitalWrite(LED3, systemOn ? LOW : HIGH);
    Serial.print("System state: ");
    Serial.println(systemOn ? "ON" : "OFF");
    lastButtonMillis = now;
  }
  lastButtonState = buttonState;

  if (!systemOn) return; // skip sensors if OFF

  // --- Vibration ---
  int vibValue = analogRead(VIB_PIN);
  bool vibDetected = vibValue > vibThreshold;

  Serial.print("Vibration: "); Serial.print(vibValue);
  if (vibDetected) Serial.print(" -> Detected!");
  Serial.println();

  // --- Ultrasonic ---
  float distance = measureDistanceCm();
  if (distance < 0) Serial.println("Ultrasonic: Out of range / no echo");
  else Serial.printf("Ultrasonic: %.2f cm\n", distance);

  // --- PIR ---
  handlePIR();

  // --- Create BLE data packet ---
  String bleData = "{";
  bleData += "\"vib\":" + String(vibValue);
  bleData += ",\"dist\":" + String(distance, 2);
  bleData += ",\"motion\":" + String(motionDetected ? 1 : 0);
  bleData += "}";

  Serial.println("BLE TX -> " + bleData);
  pCharacteristic->setValue(bleData.c_str());
  pCharacteristic->notify();  // Send BLE notification

  Serial.println("-----------------------------");
  delay(1000);
}
