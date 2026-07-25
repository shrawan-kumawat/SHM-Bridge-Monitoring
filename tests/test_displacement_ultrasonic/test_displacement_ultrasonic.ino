/*
 * ============================================================
 * Test: HC-SR04 Ultrasonic Sensor - Displacement Measurement
 * ============================================================
 * Measures distance to a reference point on the bridge.
 * Displacement = |current_distance - baseline_distance|
 * 
 * Wiring:
 *   VCC  -> 5V
 *   GND  -> GND
 *   TRIG -> Pin 9
 *   ECHO -> Pin 10
 * ============================================================
 */

const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

float baselineDistance = 0;
const int NUM_CALIBRATION_SAMPLES = 20;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  delay(1000);
  
  // Calibrate baseline
  Serial.println("Calibrating baseline distance...");
  Serial.println("Ensure sensor is in its resting position.");
  delay(2000);
  
  float sum = 0;
  int valid = 0;
  for (int i = 0; i < NUM_CALIBRATION_SAMPLES; i++) {
    float d = getDistance();
    if (d > 0 && d < 400) {
      sum += d;
      valid++;
    }
    delay(60);
  }
  
  baselineDistance = (valid > 0) ? sum / valid : 100.0;
  Serial.print("Baseline distance: ");
  Serial.print(baselineDistance, 2);
  Serial.println(" cm");
  Serial.println();
  Serial.println("Distance(cm), Displacement(cm), Status");
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if (duration == 0) return -1.0;
  
  // Speed of sound ~343 m/s at 20°C
  float distance = (duration * 0.0343) / 2.0;
  return distance;
}

float getFilteredDistance() {
  // Median filter: take 5 readings, return median
  float readings[5];
  int validCount = 0;
  
  for (int i = 0; i < 5; i++) {
    float d = getDistance();
    if (d > 0 && d < 400) {
      readings[validCount++] = d;
    }
    delay(15);
  }
  
  if (validCount == 0) return -1.0;
  
  // Simple bubble sort for median
  for (int i = 0; i < validCount - 1; i++) {
    for (int j = 0; j < validCount - i - 1; j++) {
      if (readings[j] > readings[j + 1]) {
        float temp = readings[j];
        readings[j] = readings[j + 1];
        readings[j + 1] = temp;
      }
    }
  }
  
  return readings[validCount / 2];
}

void loop() {
  float distance = getFilteredDistance();
  
  if (distance > 0) {
    float displacement = abs(distance - baselineDistance);
    
    Serial.print(distance, 2);
    Serial.print(", ");
    Serial.print(displacement, 3);
    Serial.print(", ");
    
    if (displacement < 0.5) {
      Serial.println("NORMAL");
    } else if (displacement < 1.5) {
      Serial.println("WARNING");
    } else {
      Serial.println("DANGER");
    }
  } else {
    Serial.println("ERROR: No echo received");
  }
  
  delay(500);
}
