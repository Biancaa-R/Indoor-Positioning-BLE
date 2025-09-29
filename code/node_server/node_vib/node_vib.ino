/*
  Vibration (analog AO) on GPIO36 (A0)
  HC-SR04 ultrasonic: TRIG -> GPIO4, ECHO -> GPIO16 (use level shifter / voltage divider)

  - Prints analog vibration value and a simple threshold-based detection.
  - Measures distance with HC-SR04 and prints it (cm).
*/

const int vibPin = 36;     // A0 on ESP32 (GPIO36)
const int trigPin = 4;     // TRIG for ultrasonic
const int echoPin = 16;    // ECHO for ultrasonic (use voltage divider!)

const int vibThreshold = 500;   // tune this by observing raw analog values
const unsigned long ultrasonicTimeout = 30000UL; // microseconds (30 ms -> ~5 m)

// Helper to measure distance in cm
float measureDistanceCm() {
  // Send trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // 10 us pulse
  digitalWrite(trigPin, LOW);

  // Read echo pulse duration (timeout protects from blocking)
  unsigned long duration = pulseIn(echoPin, HIGH, ultrasonicTimeout);

  if (duration == 0) {
    // no echo (out of range or blocked)
    return -1.0;
  }

  // Sound speed: 343 m/s => 0.0343 cm/us, divide by 2 for round trip
  float distanceCm = (duration * 0.0343f) / 2.0f;
  return distanceCm;
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT); // ECHO must be level-shifted to safe voltage
  digitalWrite(trigPin, LOW);
  delay(50);
}

void loop() {
  // --- Vibration (analog) ---
  int vibValue = analogRead(vibPin); // 0 - 4095 on ESP32 (12-bit)
  Serial.print("Vib: ");
  Serial.print(vibValue);

  if (vibValue > vibThreshold) {
    Serial.print("  -> Detected vibration");
  } else {
    Serial.print("  -> ...");
  }
  Serial.println();

  // --- Ultrasonic distance ---
  float distance = measureDistanceCm();
  if (distance < 0) {
    Serial.println("Ultrasonic: Out of range / no echo");
  } else {
    Serial.print("Ultrasonic: ");
    Serial.print(distance, 1);
    Serial.println(" cm");
  }

  Serial.println("---------------------------");
  delay(150); // adjust as needed
}