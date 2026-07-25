/*
 * ============================================================
 * Test: Optical Fiber Sensor - Strain Detection
 * ============================================================
 * Simulates Fiber Bragg Grating (FBG) strain measurement.
 * 
 * Working Principle:
 *   In production FBG sensors, light passes through an optical
 *   fiber with periodic refractive index variations (gratings).
 *   When the fiber is strained (stretched/compressed), the 
 *   grating period changes, causing a shift in the reflected
 *   wavelength. This wavelength shift is proportional to strain.
 * 
 * Prototype Implementation:
 *   Uses a phototransistor/photodiode at one end of an optical
 *   fiber, with an LED source at the other end. Bending the
 *   fiber changes light transmission intensity, simulating
 *   strain-induced signal changes.
 * 
 * Wiring:
 *   Photodiode/Phototransistor output -> A0 (with pull-down resistor)
 *   LED source -> 5V (with current-limiting resistor)
 * ============================================================
 */

const int OPTICAL_PIN = A0;

// Calibration values
int baseline = 0;
const int NUM_CALIBRATION = 50;

// Running average filter
const int FILTER_SIZE = 20;
int filterBuffer[FILTER_SIZE];
int filterIndex = 0;
bool filterReady = false;

// Strain thresholds (in ADC deviation from baseline)
const int STRAIN_WARNING = 100;   // ~10% deviation
const int STRAIN_DANGER  = 250;   // ~25% deviation

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("Optical Fiber Strain Sensor Test");
  Serial.println("================================");
  Serial.println();
  
  // Allow LED source to stabilize
  Serial.println("Waiting for light source to stabilize...");
  delay(2000);
  
  // Calibrate baseline (no strain condition)
  Serial.println("Calibrating baseline (no strain)...");
  long sum = 0;
  for (int i = 0; i < NUM_CALIBRATION; i++) {
    sum += analogRead(OPTICAL_PIN);
    delay(20);
  }
  baseline = sum / NUM_CALIBRATION;
  
  Serial.print("Baseline ADC value: ");
  Serial.println(baseline);
  Serial.print("Baseline voltage: ");
  Serial.print(baseline * 5.0 / 1023.0, 3);
  Serial.println(" V");
  Serial.println();
  
  // Initialize filter
  for (int i = 0; i < FILTER_SIZE; i++) filterBuffer[i] = baseline;
  
  Serial.println("RawADC, FilteredADC, Deviation, Strain(%), Voltage(V), Status");
}

int getFilteredReading() {
  int raw = analogRead(OPTICAL_PIN);
  
  filterBuffer[filterIndex] = raw;
  filterIndex = (filterIndex + 1) % FILTER_SIZE;
  if (filterIndex == 0) filterReady = true;
  
  long sum = 0;
  int count = filterReady ? FILTER_SIZE : filterIndex;
  for (int i = 0; i < count; i++) {
    sum += filterBuffer[i];
  }
  return sum / count;
}

void loop() {
  int raw = analogRead(OPTICAL_PIN);
  int filtered = getFilteredReading();
  int deviation = abs(filtered - baseline);
  float strainPercent = (deviation / (float)baseline) * 100.0;
  float voltage = filtered * 5.0 / 1023.0;
  
  Serial.print(raw);
  Serial.print(", ");
  Serial.print(filtered);
  Serial.print(", ");
  Serial.print(deviation);
  Serial.print(", ");
  Serial.print(strainPercent, 2);
  Serial.print(", ");
  Serial.print(voltage, 3);
  Serial.print(", ");
  
  if (deviation > STRAIN_DANGER) {
    Serial.println("DANGER - Critical strain detected!");
  } else if (deviation > STRAIN_WARNING) {
    Serial.println("WARNING - Elevated strain");
  } else {
    Serial.println("NORMAL");
  }
  
  delay(200);
}
