/*
 * ============================================================
 * Structural Health Monitoring (SHM) of Bridges
 * Main Sensor Hub - Arduino Uno
 * ============================================================
 * Author: Shrawan Kumawat
 * Project: IoT-Based Bridge Health Monitoring System
 * 
 * Sensors:
 *   1. MPU6050 Accelerometer (I2C) - Vibration Detection
 *   2. HC-SR04 Ultrasonic Sensor - Displacement Measurement
 *   3. DHT11 Temperature Sensor - Thermal Expansion Monitoring
 *   4. Optical Fiber Sensor (Analog) - Strain Detection
 * 
 * Output:
 *   - 16x2 LCD Display (Status + Vibration)
 *   - LED Indicators (Green/Yellow/Red)
 *   - Serial Data to ESP8266 for ThingSpeak Upload
 * ============================================================
 */

#include <Wire.h>
#include <LiquidCrystal.h>
#include <math.h>

// ===================== PIN DEFINITIONS =====================

// LCD Pins (HD44780)
const int LCD_RS = 12;
const int LCD_EN = 11;
const int LCD_D4 = 5;
const int LCD_D5 = 4;
const int LCD_D6 = 3;
const int LCD_D7 = 2;

// LED Indicators
const int LED_GREEN  = 6;
const int LED_YELLOW = 7;
const int LED_RED    = 8;

// HC-SR04 Ultrasonic Sensor
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

// DHT11 Temperature Sensor
const int DHT_PIN = 13;

// Optical Fiber Sensor (Analog)
const int OPTICAL_FIBER_PIN = A0;

// ===================== MPU6050 REGISTERS =====================
const int MPU6050_ADDR = 0x68;
const int PWR_MGMT_1   = 0x6B;
const int ACCEL_XOUT_H = 0x3B;
const int ACCEL_CONFIG  = 0x1C;

// ===================== THRESHOLDS =====================

// Vibration thresholds (m/s²) based on IS 13317 guidelines
const float VIBRATION_SAFE     = 2.0;   // Green: Normal
const float VIBRATION_WARNING  = 4.5;   // Yellow: Warning
const float VIBRATION_DANGER   = 7.0;   // Red: Danger

// Displacement thresholds (cm)
const float DISPLACEMENT_BASELINE = 0.0;  // Will be calibrated on startup
const float DISPLACEMENT_WARNING  = 0.5;  // Warning if displacement > 0.5 cm
const float DISPLACEMENT_DANGER   = 1.5;  // Danger if displacement > 1.5 cm

// Temperature thresholds (°C)
const float TEMP_LOW_WARNING   = -10.0;
const float TEMP_HIGH_WARNING  = 55.0;
const float TEMP_DANGER        = 65.0;

// Optical Fiber strain thresholds (normalized 0-1023 ADC)
const int STRAIN_NORMAL  = 512;   // Baseline (no strain)
const int STRAIN_WARNING = 200;   // Deviation threshold for warning
const int STRAIN_DANGER  = 400;   // Deviation threshold for danger

// ===================== GLOBAL VARIABLES =====================

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Accelerometer data
float accelX, accelY, accelZ;
float vibrationMagnitude;

// Displacement
float baselineDistance = 0.0;
float currentDisplacement = 0.0;

// Temperature
float temperature = 0.0;
float humidity = 0.0;

// Optical Fiber
int opticalFiberRaw = 0;
float strainValue = 0.0;

// Overall status
enum HealthStatus { NORMAL, WARNING, DANGER };
HealthStatus overallStatus = NORMAL;

// Timing
unsigned long lastReadTime = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastSerialSend = 0;
const unsigned long READ_INTERVAL = 500;      // Read sensors every 500ms
const unsigned long LCD_INTERVAL = 1000;       // Update LCD every 1s
const unsigned long SERIAL_INTERVAL = 15000;   // Send to ESP every 15s (ThingSpeak rate limit)

// LCD display page cycling
int displayPage = 0;
const int TOTAL_PAGES = 4;

// ===================== SETUP =====================

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SHM Bridge v2.0");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  // Initialize LED pins
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  // Initialize Ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initialize DHT pin
  pinMode(DHT_PIN, INPUT);
  
  // Initialize MPU6050
  initMPU6050();
  
  // Startup LED test
  ledTest();
  
  // Calibrate baseline distance
  calibrateBaseline();
  
  delay(1000);
  lcd.clear();
  
  Serial.println("SHM System Initialized");
  Serial.println("========================");
}

// ===================== MAIN LOOP =====================

void loop() {
  unsigned long currentTime = millis();
  
  // Read all sensors at regular interval
  if (currentTime - lastReadTime >= READ_INTERVAL) {
    lastReadTime = currentTime;
    
    readAccelerometer();
    readUltrasonicDisplacement();
    readTemperature();
    readOpticalFiber();
    
    // Evaluate overall structural health
    evaluateHealth();
    
    // Update LED indicators
    updateLEDs();
  }
  
  // Update LCD display
  if (currentTime - lastLCDUpdate >= LCD_INTERVAL) {
    lastLCDUpdate = currentTime;
    updateLCD();
    displayPage = (displayPage + 1) % TOTAL_PAGES;
  }
  
  // Send data to ESP8266 via Serial
  if (currentTime - lastSerialSend >= SERIAL_INTERVAL) {
    lastSerialSend = currentTime;
    sendDataToESP();
  }
}

// ===================== MPU6050 FUNCTIONS =====================

void initMPU6050() {
  // Wake up MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0x00);  // Clear sleep bit
  Wire.endTransmission(true);
  delay(100);
  
  // Set accelerometer range to ±4g
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(ACCEL_CONFIG);
  Wire.write(0x08);  // ±4g range
  Wire.endTransmission(true);
  
  Serial.println("[INIT] MPU6050 Accelerometer initialized (±4g)");
}

void readAccelerometer() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 6, true);
  
  int16_t rawX = Wire.read() << 8 | Wire.read();
  int16_t rawY = Wire.read() << 8 | Wire.read();
  int16_t rawZ = Wire.read() << 8 | Wire.read();
  
  // Convert to m/s² (±4g range, sensitivity = 8192 LSB/g)
  accelX = (rawX / 8192.0) * 9.81;
  accelY = (rawY / 8192.0) * 9.81;
  accelZ = (rawZ / 8192.0) * 9.81;
  
  // Calculate vibration magnitude (removing gravity component)
  // Gravity is ~9.81 m/s² on Z-axis when sensor is flat
  float dynamicZ = accelZ - 9.81;
  vibrationMagnitude = sqrt(accelX * accelX + accelY * accelY + dynamicZ * dynamicZ);
}

// ===================== ULTRASONIC DISPLACEMENT =====================

float measureDistance() {
  // Send trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Measure echo duration
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);  // 30ms timeout
  
  if (duration == 0) {
    return -1.0;  // No echo received
  }
  
  // Calculate distance in cm
  // Speed of sound = 343 m/s at 20°C
  // Adjust for temperature if available
  float speedOfSound = 331.3 + (0.606 * temperature);  // m/s
  float distance = (duration * speedOfSound * 0.0001) / 2.0;  // cm
  
  return distance;
}

void calibrateBaseline() {
  Serial.println("[CALIBRATION] Measuring baseline distance...");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...  ");
  
  float sum = 0;
  int validReadings = 0;
  
  for (int i = 0; i < 10; i++) {
    float d = measureDistance();
    if (d > 0) {
      sum += d;
      validReadings++;
    }
    delay(100);
  }
  
  if (validReadings > 0) {
    baselineDistance = sum / validReadings;
    Serial.print("[CALIBRATION] Baseline distance: ");
    Serial.print(baselineDistance, 2);
    Serial.println(" cm");
  } else {
    baselineDistance = 100.0;  // Default fallback
    Serial.println("[CALIBRATION] WARNING: Using default baseline");
  }
}

void readUltrasonicDisplacement() {
  // Take average of 3 readings for noise reduction
  float sum = 0;
  int validCount = 0;
  
  for (int i = 0; i < 3; i++) {
    float d = measureDistance();
    if (d > 0 && d < 400) {  // Valid range: 0-400 cm
      sum += d;
      validCount++;
    }
    delay(10);
  }
  
  if (validCount > 0) {
    float avgDistance = sum / validCount;
    currentDisplacement = abs(avgDistance - baselineDistance);
  }
}

// ===================== TEMPERATURE SENSOR (DHT11) =====================

/*
 * Simplified DHT11 reading without external library
 * DHT11 uses single-wire protocol
 */
void readTemperature() {
  uint8_t data[5] = {0};
  
  // Send start signal
  pinMode(DHT_PIN, OUTPUT);
  digitalWrite(DHT_PIN, LOW);
  delay(18);
  digitalWrite(DHT_PIN, HIGH);
  delayMicroseconds(40);
  pinMode(DHT_PIN, INPUT);
  
  // Wait for DHT response
  unsigned long timeout = micros();
  while (digitalRead(DHT_PIN) == LOW) {
    if (micros() - timeout > 100) return;
  }
  timeout = micros();
  while (digitalRead(DHT_PIN) == HIGH) {
    if (micros() - timeout > 100) return;
  }
  
  // Read 40 bits (5 bytes)
  for (int i = 0; i < 40; i++) {
    timeout = micros();
    while (digitalRead(DHT_PIN) == LOW) {
      if (micros() - timeout > 100) return;
    }
    
    unsigned long startTime = micros();
    
    timeout = micros();
    while (digitalRead(DHT_PIN) == HIGH) {
      if (micros() - timeout > 100) return;
    }
    
    unsigned long pulseWidth = micros() - startTime;
    
    // If pulse > 40µs, it's a '1'
    if (pulseWidth > 40) {
      data[i / 8] |= (1 << (7 - (i % 8)));
    }
  }
  
  // Verify checksum
  uint8_t checksum = data[0] + data[1] + data[2] + data[3];
  if (checksum == data[4]) {
    humidity = data[0] + data[1] * 0.1;
    temperature = data[2] + data[3] * 0.1;
  }
}

// ===================== OPTICAL FIBER SENSOR =====================

/*
 * Optical Fiber Sensor (Analog Input)
 * 
 * Simulates Fiber Bragg Grating (FBG) behavior:
 * - Light intensity changes indicate strain/deformation
 * - Baseline value (~512) represents no strain
 * - Deviation from baseline indicates structural stress
 * 
 * In production: Would use interrogator + FBG sensor
 * For prototype: Photodiode/phototransistor reading light intensity
 * through optical fiber
 */
void readOpticalFiber() {
  // Take multiple readings and average for stability
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(OPTICAL_FIBER_PIN);
    delay(2);
  }
  opticalFiberRaw = sum / 10;
  
  // Calculate strain as deviation from baseline
  // Normalize to percentage (0-100%)
  strainValue = abs(opticalFiberRaw - STRAIN_NORMAL) / 512.0 * 100.0;
  strainValue = constrain(strainValue, 0.0, 100.0);
}

// ===================== HEALTH EVALUATION =====================

void evaluateHealth() {
  overallStatus = NORMAL;
  
  // Check vibration
  if (vibrationMagnitude > VIBRATION_DANGER) {
    overallStatus = DANGER;
  } else if (vibrationMagnitude > VIBRATION_WARNING) {
    if (overallStatus < WARNING) overallStatus = WARNING;
  }
  
  // Check displacement
  if (currentDisplacement > DISPLACEMENT_DANGER) {
    overallStatus = DANGER;
  } else if (currentDisplacement > DISPLACEMENT_WARNING) {
    if (overallStatus < WARNING) overallStatus = WARNING;
  }
  
  // Check temperature
  if (temperature > TEMP_DANGER || temperature < TEMP_LOW_WARNING) {
    overallStatus = DANGER;
  } else if (temperature > TEMP_HIGH_WARNING) {
    if (overallStatus < WARNING) overallStatus = WARNING;
  }
  
  // Check optical fiber strain
  int strainDeviation = abs(opticalFiberRaw - STRAIN_NORMAL);
  if (strainDeviation > STRAIN_DANGER) {
    overallStatus = DANGER;
  } else if (strainDeviation > STRAIN_WARNING) {
    if (overallStatus < WARNING) overallStatus = WARNING;
  }
}

// ===================== LED CONTROL =====================

void updateLEDs() {
  // Turn off all LEDs first
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  
  switch (overallStatus) {
    case NORMAL:
      digitalWrite(LED_GREEN, HIGH);
      break;
    case WARNING:
      digitalWrite(LED_YELLOW, HIGH);
      break;
    case DANGER:
      digitalWrite(LED_RED, HIGH);
      break;
  }
}

void ledTest() {
  digitalWrite(LED_GREEN, HIGH);
  delay(300);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  delay(300);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  delay(300);
  digitalWrite(LED_RED, LOW);
}

// ===================== LCD DISPLAY =====================

void updateLCD() {
  lcd.clear();
  
  switch (displayPage) {
    case 0:  // Vibration + Status
      lcd.setCursor(0, 0);
      lcd.print("Vib:");
      lcd.print(vibrationMagnitude, 2);
      lcd.print(" m/s2");
      lcd.setCursor(0, 1);
      lcd.print("Status:");
      printStatus();
      break;
      
    case 1:  // Displacement
      lcd.setCursor(0, 0);
      lcd.print("Displacement:");
      lcd.setCursor(0, 1);
      lcd.print(currentDisplacement, 2);
      lcd.print(" cm ");
      printStatusShort();
      break;
      
    case 2:  // Temperature
      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.print(temperature, 1);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("Humidity:");
      lcd.print(humidity, 0);
      lcd.print("%");
      break;
      
    case 3:  // Optical Fiber Strain
      lcd.setCursor(0, 0);
      lcd.print("Strain:");
      lcd.print(strainValue, 1);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("Fiber:");
      lcd.print(opticalFiberRaw);
      lcd.print(" ");
      printStatusShort();
      break;
  }
}

void printStatus() {
  switch (overallStatus) {
    case NORMAL:  lcd.print("Normal "); break;
    case WARNING: lcd.print("WARNING"); break;
    case DANGER:  lcd.print("DANGER!"); break;
  }
}

void printStatusShort() {
  switch (overallStatus) {
    case NORMAL:  lcd.print("[OK]"); break;
    case WARNING: lcd.print("[!!]"); break;
    case DANGER:  lcd.print("[XX]"); break;
  }
}

// ===================== SERIAL DATA TO ESP8266 =====================

/*
 * Sends sensor data as comma-separated values to ESP8266
 * Format: accelX,accelY,accelZ,vibMag,displacement,temperature,humidity,strain,status
 */
void sendDataToESP() {
  Serial.print("DATA:");
  Serial.print(accelX, 4);    Serial.print(",");
  Serial.print(accelY, 4);    Serial.print(",");
  Serial.print(accelZ, 4);    Serial.print(",");
  Serial.print(vibrationMagnitude, 4);  Serial.print(",");
  Serial.print(currentDisplacement, 3); Serial.print(",");
  Serial.print(temperature, 2);  Serial.print(",");
  Serial.print(humidity, 1);     Serial.print(",");
  Serial.print(strainValue, 2);  Serial.print(",");
  Serial.println(overallStatus);
  
  // Also print human-readable format for debugging
  Serial.println("--- SHM Sensor Readings ---");
  Serial.print("  Accel (m/s2): X="); Serial.print(accelX, 2);
  Serial.print(" Y="); Serial.print(accelY, 2);
  Serial.print(" Z="); Serial.println(accelZ, 2);
  Serial.print("  Vibration Magnitude: "); Serial.print(vibrationMagnitude, 2); Serial.println(" m/s2");
  Serial.print("  Displacement: "); Serial.print(currentDisplacement, 2); Serial.println(" cm");
  Serial.print("  Temperature: "); Serial.print(temperature, 1); Serial.print("C  Humidity: "); Serial.print(humidity, 0); Serial.println("%");
  Serial.print("  Optical Fiber Strain: "); Serial.print(strainValue, 1); Serial.print("% (Raw: "); Serial.print(opticalFiberRaw); Serial.println(")");
  Serial.print("  Overall Status: ");
  switch (overallStatus) {
    case NORMAL:  Serial.println("NORMAL (GREEN)"); break;
    case WARNING: Serial.println("WARNING (YELLOW)"); break;
    case DANGER:  Serial.println("DANGER (RED)"); break;
  }
  Serial.println("---------------------------");
}
