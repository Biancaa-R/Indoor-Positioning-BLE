#include <WiFi.h>
#include <HTTPClient.h>

// --- Pins ---
#define LED2      15
#define BUTTON    0
#define VIB_PIN   36
#define TRIG_PIN  4
#define ECHO_PIN  16
#define PIR_PIN   27
#define LED_PIR   26

// --- WiFi / ThingSpeak ---
const char* ssid = "Xiaomi 11i";
const char* password = "timetime";
const char* apiKey = "LLXF7DF4L239D0VH";

// --- Thresholds / timers ---
const int vibThreshold = 300;
const unsigned long ultrasonicTimeout = 30000UL;
const int pirHoldSeconds = 10;
const unsigned long tsInterval = 15000; // 15 sec

// --- System state ---
int lastButtonState = HIGH;
unsigned long lastButtonMillis = 0;
bool systemOn = false;

// --- PIR state ---
volatile bool pirTriggered = false;
bool motionDetected = false;
unsigned long lastPirTrigger = 0;

// --- ThingSpeak timer ---
unsigned long lastThingSpeakMillis = 0;

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

// --- ThingSpeak upload ---
void sendToThingSpeak(int vibValue, float distance, bool motion) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String("http://api.thingspeak.com/update?api_key=") + apiKey +
                 "&field1=" + String(vibValue) +
                 "&field2=" + String(distance, 2) +
                 "&field3=" + String(motion ? 1 : 0);
    http.begin(url);
    int code = http.GET();
    if (code > 0) Serial.println("ThingSpeak updated!");
    else Serial.println("Error sending data: " + String(code));
    http.end();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED2, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  pinMode(VIB_PIN, INPUT); // NO PULLUP
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(PIR_PIN, INPUT); // NO PULLUP
  pinMode(LED_PIR, OUTPUT);
  digitalWrite(LED_PIR, LOW);

  attachInterrupt(digitalPinToInterrupt(PIR_PIN), detectsMovement, RISING);
  digitalWrite(TRIG_PIN, LOW);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void loop() {
  unsigned long now = millis();

  // --- Button toggle ---
  int buttonState = digitalRead(BUTTON);
  if (buttonState == LOW && lastButtonState == HIGH && (now - lastButtonMillis > 200)) {
    systemOn = !systemOn;
    digitalWrite(LED2, systemOn ? HIGH : LOW);
    Serial.print("System state: ");
    Serial.println(systemOn ? "ON" : "OFF");
    lastButtonMillis = now;
  }
  lastButtonState = buttonState;

  if (!systemOn) return; // skip sensors if OFF

  // --- Vibration ---
  int vibValue = analogRead(VIB_PIN);
  Serial.print("Vibration: "); Serial.print(vibValue);
  if (vibValue > vibThreshold) Serial.print(" -> Detected!");
  Serial.println();

  // --- Ultrasonic ---
  float distance = measureDistanceCm();
  if (distance < 0) Serial.println("Ultrasonic: Out of range / no echo");
  else {
    Serial.print("Ultrasonic: "); Serial.print(distance, 1); Serial.println(" cm");
  }

  // --- PIR ---
  handlePIR();
  int pirValue = motionDetected ? 1 : 0;

  // --- ThingSpeak (non-blocking 15s) ---
  if (now - lastThingSpeakMillis > tsInterval) {
    sendToThingSpeak(vibValue, distance, motionDetected);
    lastThingSpeakMillis = now;
  }

  Serial.println("-----------------------------");
  delay(200); // short delay for stability
}
