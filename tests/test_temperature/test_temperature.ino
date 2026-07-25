/*
 * ============================================================
 * Test: DHT11 Temperature & Humidity Sensor
 * ============================================================
 * Monitors thermal expansion/contraction of bridge materials.
 * Temperature changes cause steel/concrete to expand or contract,
 * affecting structural integrity.
 * 
 * Wiring:
 *   VCC  -> 5V
 *   GND  -> GND
 *   DATA -> Pin 13 (with 10K pull-up resistor to VCC)
 * ============================================================
 */

const int DHT_PIN = 13;

float temperature = 0;
float humidity = 0;

// Store temperature history for trend analysis
float tempHistory[10];
int historyIndex = 0;
bool historyFull = false;

void setup() {
  Serial.begin(9600);
  pinMode(DHT_PIN, INPUT);
  
  delay(2000);  // DHT11 needs 1-2s to stabilize
  
  Serial.println("DHT11 Temperature Sensor Test");
  Serial.println("Monitoring thermal conditions for SHM");
  Serial.println();
  Serial.println("Temp(C), Humidity(%), TempChange(C/min), Status");
  
  // Initialize history
  for (int i = 0; i < 10; i++) tempHistory[i] = 0;
}

bool readDHT11() {
  uint8_t data[5] = {0};
  
  // Start signal
  pinMode(DHT_PIN, OUTPUT);
  digitalWrite(DHT_PIN, LOW);
  delay(18);
  digitalWrite(DHT_PIN, HIGH);
  delayMicroseconds(40);
  pinMode(DHT_PIN, INPUT);
  
  // Wait for response
  unsigned long timeout = micros();
  while (digitalRead(DHT_PIN) == LOW) {
    if (micros() - timeout > 200) return false;
  }
  timeout = micros();
  while (digitalRead(DHT_PIN) == HIGH) {
    if (micros() - timeout > 200) return false;
  }
  
  // Read 40 bits
  for (int i = 0; i < 40; i++) {
    timeout = micros();
    while (digitalRead(DHT_PIN) == LOW) {
      if (micros() - timeout > 100) return false;
    }
    unsigned long start = micros();
    timeout = micros();
    while (digitalRead(DHT_PIN) == HIGH) {
      if (micros() - timeout > 100) return false;
    }
    if (micros() - start > 40) {
      data[i / 8] |= (1 << (7 - (i % 8)));
    }
  }
  
  // Checksum
  if (data[4] != (data[0] + data[1] + data[2] + data[3])) {
    return false;
  }
  
  humidity = data[0] + data[1] * 0.1;
  temperature = data[2] + data[3] * 0.1;
  return true;
}

float calculateTempRate() {
  if (!historyFull && historyIndex < 2) return 0;
  
  int count = historyFull ? 10 : historyIndex;
  float oldest = tempHistory[(historyIndex - count + 10) % 10];
  float newest = tempHistory[(historyIndex - 1 + 10) % 10];
  
  // Rate of change per minute (readings every 2 seconds, 10 readings = 20 seconds)
  float timePeriodMinutes = (count * 2.0) / 60.0;
  return (newest - oldest) / timePeriodMinutes;
}

void loop() {
  if (readDHT11()) {
    // Store in history
    tempHistory[historyIndex] = temperature;
    historyIndex = (historyIndex + 1) % 10;
    if (historyIndex == 0) historyFull = true;
    
    float tempRate = calculateTempRate();
    
    Serial.print(temperature, 1);
    Serial.print(", ");
    Serial.print(humidity, 1);
    Serial.print(", ");
    Serial.print(tempRate, 2);
    Serial.print(", ");
    
    // Evaluate thermal status
    if (temperature > 65 || temperature < -10) {
      Serial.println("DANGER - Extreme temperature!");
    } else if (temperature > 55 || abs(tempRate) > 5.0) {
      Serial.println("WARNING - High temp or rapid change");
    } else {
      Serial.println("NORMAL");
    }
  } else {
    Serial.println("ERROR: DHT11 read failed");
  }
  
  delay(2000);  // DHT11 minimum sampling period
}
