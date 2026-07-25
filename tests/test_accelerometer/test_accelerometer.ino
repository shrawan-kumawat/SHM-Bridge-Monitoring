/*
 * ============================================================
 * Test: MPU6050 Accelerometer - Vibration Detection
 * ============================================================
 * Wiring:
 *   VCC -> 5V
 *   GND -> GND
 *   SDA -> A4
 *   SCL -> A5
 * ============================================================
 */

#include <Wire.h>

const int MPU6050_ADDR = 0x68;

float accelX, accelY, accelZ;
float offsetX = 0, offsetY = 0, offsetZ = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  // Wake up MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  
  // Set ±4g range
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x1C);
  Wire.write(0x08);
  Wire.endTransmission(true);
  
  delay(1000);
  
  // Calibrate offsets (keep sensor still during startup)
  Serial.println("Calibrating accelerometer... Keep sensor still!");
  calibrate();
  Serial.println("Calibration complete.");
  Serial.println("X(m/s2), Y(m/s2), Z(m/s2), Magnitude(m/s2)");
}

void calibrate() {
  float sumX = 0, sumY = 0, sumZ = 0;
  for (int i = 0; i < 100; i++) {
    readRaw();
    sumX += accelX;
    sumY += accelY;
    sumZ += accelZ;
    delay(10);
  }
  offsetX = sumX / 100.0;
  offsetY = sumY / 100.0;
  offsetZ = (sumZ / 100.0) - 9.81;  // Remove gravity
}

void readRaw() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 6, true);
  
  int16_t rawX = Wire.read() << 8 | Wire.read();
  int16_t rawY = Wire.read() << 8 | Wire.read();
  int16_t rawZ = Wire.read() << 8 | Wire.read();
  
  accelX = (rawX / 8192.0) * 9.81;
  accelY = (rawY / 8192.0) * 9.81;
  accelZ = (rawZ / 8192.0) * 9.81;
}

void loop() {
  readRaw();
  
  float corrX = accelX - offsetX;
  float corrY = accelY - offsetY;
  float corrZ = accelZ - offsetZ;
  
  float magnitude = sqrt(corrX*corrX + corrY*corrY + corrZ*corrZ);
  
  Serial.print(corrX, 3); Serial.print(", ");
  Serial.print(corrY, 3); Serial.print(", ");
  Serial.print(corrZ, 3); Serial.print(", ");
  Serial.println(magnitude, 3);
  
  delay(100);
}
