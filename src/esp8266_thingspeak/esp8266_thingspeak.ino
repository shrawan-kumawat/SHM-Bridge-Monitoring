/*
 * ============================================================
 * ESP8266 - ThingSpeak Cloud Upload Module
 * Structural Health Monitoring of Bridges
 * ============================================================
 * Author: Shrawan Kumawat
 * 
 * Description:
 *   Receives sensor data from Arduino Uno via Serial (UART)
 *   and uploads it to ThingSpeak IoT platform for real-time
 *   cloud monitoring and visualization.
 * 
 * ThingSpeak Channel Fields:
 *   Field 1: Acceleration X (m/s²)
 *   Field 2: Acceleration Y (m/s²)
 *   Field 3: Acceleration Z (m/s²)
 *   Field 4: Vibration Magnitude (m/s²)
 *   Field 5: Displacement (cm)
 *   Field 6: Temperature (°C)
 *   Field 7: Optical Fiber Strain (%)
 *   Field 8: Health Status (0=Normal, 1=Warning, 2=Danger)
 * 
 * Wiring:
 *   ESP8266 RX -> Arduino TX (via voltage divider 5V->3.3V)
 *   ESP8266 TX -> Arduino RX
 *   ESP8266 VCC -> 3.3V
 *   ESP8266 GND -> GND
 *   ESP8266 CH_PD -> 3.3V (pull-up)
 * ============================================================
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// ===================== CONFIGURATION =====================

// WiFi Credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ThingSpeak Configuration
const char* THINGSPEAK_HOST = "api.thingspeak.com";
const String THINGSPEAK_API_KEY = "YOUR_WRITE_API_KEY";
const unsigned long CHANNEL_ID = 0;  // Your channel ID

// ThingSpeak requires minimum 15 seconds between updates
const unsigned long UPLOAD_INTERVAL = 16000;

// ===================== GLOBAL VARIABLES =====================

// Sensor data received from Arduino
float accelX = 0, accelY = 0, accelZ = 0;
float vibrationMag = 0;
float displacement = 0;
float temperature = 0;
float humidity = 0;
float strain = 0;
int healthStatus = 0;

// Timing
unsigned long lastUpload = 0;
bool newDataAvailable = false;

// Serial buffer
String serialBuffer = "";

// WiFi connection status
bool wifiConnected = false;

// ===================== SETUP =====================

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  Serial.println();
  Serial.println("================================");
  Serial.println("SHM Bridge - ESP8266 Cloud Module");
  Serial.println("================================");
  
  // Connect to WiFi
  connectWiFi();
  
  Serial.println("[READY] Waiting for sensor data from Arduino...");
}

// ===================== MAIN LOOP =====================

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    connectWiFi();
  }
  
  // Read serial data from Arduino
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      processSerialData(serialBuffer);
      serialBuffer = "";
    } else if (c != '\r') {
      serialBuffer += c;
    }
  }
  
  // Upload data to ThingSpeak at regular intervals
  unsigned long currentTime = millis();
  if (newDataAvailable && (currentTime - lastUpload >= UPLOAD_INTERVAL)) {
    if (wifiConnected) {
      uploadToThingSpeak();
      lastUpload = currentTime;
      newDataAvailable = false;
    }
  }
}

// ===================== WiFi CONNECTION =====================

void connectWiFi() {
  Serial.print("[WiFi] Connecting to ");
  Serial.print(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println();
    Serial.print("[WiFi] Connected! IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WiFi] Signal Strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    wifiConnected = false;
    Serial.println();
    Serial.println("[WiFi] Connection FAILED. Will retry...");
  }
}

// ===================== SERIAL DATA PARSING =====================

/*
 * Expected format from Arduino:
 * DATA:accelX,accelY,accelZ,vibMag,displacement,temperature,humidity,strain,status
 */
void processSerialData(String data) {
  if (!data.startsWith("DATA:")) {
    return;  // Ignore non-data messages
  }
  
  // Remove "DATA:" prefix
  data = data.substring(5);
  
  // Parse comma-separated values
  int fieldIndex = 0;
  int startPos = 0;
  
  for (int i = 0; i <= data.length(); i++) {
    if (i == data.length() || data.charAt(i) == ',') {
      String value = data.substring(startPos, i);
      
      switch (fieldIndex) {
        case 0: accelX = value.toFloat(); break;
        case 1: accelY = value.toFloat(); break;
        case 2: accelZ = value.toFloat(); break;
        case 3: vibrationMag = value.toFloat(); break;
        case 4: displacement = value.toFloat(); break;
        case 5: temperature = value.toFloat(); break;
        case 6: humidity = value.toFloat(); break;
        case 7: strain = value.toFloat(); break;
        case 8: healthStatus = value.toInt(); break;
      }
      
      fieldIndex++;
      startPos = i + 1;
    }
  }
  
  if (fieldIndex >= 9) {
    newDataAvailable = true;
    Serial.println("[DATA] Received sensor data from Arduino");
  }
}

// ===================== THINGSPEAK UPLOAD =====================

void uploadToThingSpeak() {
  WiFiClient client;
  HTTPClient http;
  
  // Build ThingSpeak update URL
  String url = "http://";
  url += THINGSPEAK_HOST;
  url += "/update?api_key=";
  url += THINGSPEAK_API_KEY;
  url += "&field1=" + String(accelX, 4);
  url += "&field2=" + String(accelY, 4);
  url += "&field3=" + String(accelZ, 4);
  url += "&field4=" + String(vibrationMag, 4);
  url += "&field5=" + String(displacement, 3);
  url += "&field6=" + String(temperature, 2);
  url += "&field7=" + String(strain, 2);
  url += "&field8=" + String(healthStatus);
  
  // Add status message
  String statusMsg;
  switch (healthStatus) {
    case 0: statusMsg = "NORMAL"; break;
    case 1: statusMsg = "WARNING"; break;
    case 2: statusMsg = "DANGER"; break;
    default: statusMsg = "UNKNOWN"; break;
  }
  url += "&status=" + statusMsg;
  
  Serial.print("[ThingSpeak] Uploading data... ");
  
  http.begin(client, url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String response = http.getString();
    if (response.toInt() > 0) {
      Serial.print("SUCCESS (Entry #");
      Serial.print(response);
      Serial.println(")");
    } else {
      Serial.println("FAILED (Rate limited or invalid key)");
    }
  } else {
    Serial.print("HTTP ERROR: ");
    Serial.println(httpCode);
  }
  
  http.end();
  
  // Log uploaded values
  Serial.println("[ThingSpeak] Uploaded Values:");
  Serial.print("  Accel: X="); Serial.print(accelX, 2);
  Serial.print(" Y="); Serial.print(accelY, 2);
  Serial.print(" Z="); Serial.println(accelZ, 2);
  Serial.print("  Vibration: "); Serial.print(vibrationMag, 2); Serial.println(" m/s2");
  Serial.print("  Displacement: "); Serial.print(displacement, 2); Serial.println(" cm");
  Serial.print("  Temperature: "); Serial.print(temperature, 1); Serial.println(" C");
  Serial.print("  Strain: "); Serial.print(strain, 1); Serial.println(" %");
  Serial.print("  Status: "); Serial.println(statusMsg);
}
