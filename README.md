# IoT-Based Structural Health Monitoring of Bridges

> **A real-time bridge health monitoring system using multi-sensor fusion, Arduino Uno, ESP8266, and ThingSpeak for continuous structural monitoring and early warning of potential failures.**

![Arduino](https://img.shields.io/badge/Arduino-Uno-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![ESP8266](https://img.shields.io/badge/ESP8266-WiFi-E7352C?style=for-the-badge)
![C++](https://img.shields.io/badge/C%2B%2B-Programming-blue?style=for-the-badge&logo=cplusplus)
![ThingSpeak](https://img.shields.io/badge/ThingSpeak-IoT-orange?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

---

## Overview

Structural Health Monitoring (SHM) plays a vital role in ensuring the safety and longevity of bridge infrastructure. This project presents a low-cost IoT-based SHM prototype that continuously monitors critical structural parameters using multiple sensors and uploads the collected data to the ThingSpeak cloud platform for real-time visualization.

The system combines vibration sensing, displacement measurement, environmental monitoring, and optical-fiber-inspired strain detection to provide a comprehensive assessment of bridge health. Sensor data is processed by an Arduino Uno, displayed locally on a 16×2 LCD, and transmitted through an ESP8266 Wi-Fi module for cloud-based monitoring.

This prototype demonstrates how affordable embedded systems and IoT technologies can be used to implement an early-warning system for structural monitoring applications.

---

## Features

- Multi-sensor bridge health monitoring
- Real-time vibration analysis using MPU6050
- Bridge displacement measurement using HC-SR04
- Temperature and humidity monitoring using DHT11
- Optical fiber-inspired strain sensing prototype
- Local LCD status display
- Three-level LED warning system (Normal, Warning, Danger)
- Wi-Fi connectivity using ESP8266
- ThingSpeak cloud integration
- Modular standalone sensor test programs

---

## Problem Statement

Bridges are critical components of transportation infrastructure and require continuous monitoring to ensure structural safety. Conventional inspection methods rely heavily on periodic manual inspections, which can fail to detect early signs of structural deterioration.

The **Morbi Bridge collapse (October 2022)**, which resulted in the deaths of more than 140 people, highlighted the importance of implementing continuous Structural Health Monitoring (SHM) systems capable of providing real-time information about the condition of bridge structures.

This project demonstrates a low-cost IoT-based SHM prototype that combines multiple sensors with cloud connectivity to detect abnormal structural conditions and provide early warning alerts.

---

## Objectives

- Develop a low-cost Structural Health Monitoring (SHM) prototype.
- Continuously monitor bridge vibration, displacement, temperature, and strain.
- Process sensor data using an Arduino Uno.
- Display structural health status locally on a 16×2 LCD.
- Transmit monitoring data to the ThingSpeak cloud platform using ESP8266.
- Provide three-level health alerts (Normal, Warning, Danger).
- Demonstrate the feasibility of IoT-enabled bridge monitoring for smart infrastructure applications.

---

# System Architecture

The proposed Structural Health Monitoring (SHM) system consists of three major layers:

1. **Sensor Layer** – Collects structural and environmental parameters.
2. **Edge Processing Layer** – Arduino Uno performs sensor acquisition, threshold evaluation, and local visualization.
3. **Cloud Layer** – ESP8266 uploads processed data to ThingSpeak for remote monitoring.

```
┌──────────────────────────────────────────────────────────────┐
│                        SENSOR LAYER                          │
│                                                              │
│  MPU6050      HC-SR04        DHT11       Optical Fiber       │
│ Accelerometer Ultrasonic  Temp/Humidity  Strain Prototype    │
└───────────────┬──────────────┬──────────────┬─────────────────┘
                │              │              │
                ▼              ▼              ▼
┌──────────────────────────────────────────────────────────────┐
│                    EDGE PROCESSING LAYER                     │
│                                                              │
│                  Arduino Uno (ATmega328P)                   │
│                                                              │
│ • Sensor Acquisition                                         │
│ • Sensor Fusion                                              │
│ • Threshold Evaluation                                       │
│ • LCD Display                                                │
│ • LED Status Indicators                                      │
│ • UART Communication                                         │
└──────────────────────────────┬───────────────────────────────┘
                               │
                         UART (9600 baud)
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│                  ESP8266 Wi-Fi MODULE                        │
│                                                              │
│ • Wi-Fi Connectivity                                         │
│ • HTTP Client                                                │
│ • ThingSpeak REST API                                        │
└──────────────────────────────┬───────────────────────────────┘
                               │
                          Internet
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│                     CLOUD LAYER                              │
│                                                              │
│                 ThingSpeak IoT Platform                      │
│                                                              │
│ • Real-time Dashboard                                        │
│ • Data Logging                                               │
│ • Trend Visualization                                        │
│ • Remote Monitoring                                          │
└──────────────────────────────────────────────────────────────┘
```

### Data Flow

```
Sensors
    ↓
Arduino Uno
    ↓
Sensor Processing
    ↓
Health Evaluation
    ↓
LCD + LEDs
    ↓
ESP8266
    ↓
ThingSpeak Cloud
    ↓
Remote Monitoring Dashboard
```

---

# Sensors and Working Principle

The SHM system integrates multiple sensors to monitor different structural and environmental parameters. Each sensor contributes unique information for evaluating the bridge's health.

| Sensor | Parameter Measured | Purpose in SHM |
|---------|-------------------|----------------|
| **MPU6050 Accelerometer** | X, Y, Z Acceleration (m/s²) | Detects abnormal vibrations, impacts, and dynamic structural responses. |
| **HC-SR04 Ultrasonic Sensor** | Distance / Displacement (cm) | Measures displacement relative to a calibrated baseline to detect structural movement. |
| **DHT11 Temperature & Humidity Sensor** | Temperature (°C), Humidity (%) | Monitors environmental conditions that influence thermal expansion and contraction of bridge materials. |
| **Optical Fiber Prototype** | Relative Light Intensity (Strain Proxy) | Demonstrates deformation sensing using an optical fiber prototype inspired by the Fiber Bragg Grating (FBG) principle. |

---

## Sensor Operation

### MPU6050 Accelerometer

The MPU6050 continuously measures acceleration along the X, Y, and Z axes. The Arduino calculates the overall vibration magnitude, which is compared with predefined thresholds to identify abnormal structural vibrations.

---

### HC-SR04 Ultrasonic Sensor

The ultrasonic sensor measures the distance between the sensor and a reference surface. Any deviation from the calibrated baseline distance is interpreted as structural displacement.

---

### DHT11 Temperature & Humidity Sensor

Temperature variations can cause bridge materials such as steel and concrete to expand or contract. Monitoring environmental conditions helps correlate structural behavior with thermal effects.

---

### Optical Fiber Prototype

The prototype consists of an LED light source, an optical fiber, and a photodiode/phototransistor. Mechanical deformation changes the amount of transmitted light, producing an analog signal proportional to the deformation.

> **Note:** This prototype demonstrates the sensing concept inspired by Fiber Bragg Grating (FBG) technology. It measures relative light intensity rather than wavelength shift, making it suitable for educational and proof-of-concept applications.

---

---

# Hardware Requirements

| Component | Quantity |
|-----------|:--------:|
| Arduino Uno (ATmega328P) | 1 |
| ESP8266 Wi-Fi Module (NodeMCU or ESP-01) | 1 |
| MPU6050 Accelerometer/Gyroscope | 1 |
| HC-SR04 Ultrasonic Sensor | 1 |
| DHT11 Temperature & Humidity Sensor | 1 |
| Optical Fiber Prototype (LED + Photodiode/Phototransistor) | 1 |
| 16×2 LCD Display (HD44780) | 1 |
| Green LED | 1 |
| Yellow LED | 1 |
| Red LED | 1 |
| 220Ω Resistors | 4 |
| 10kΩ Potentiometer | 1 |
| Breadboard | 1 |
| Jumper Wires | As Required |
| USB Cable | 1 |

---

# Circuit Connections

## Arduino Uno Pin Assignment

| Arduino Pin | Connected Device |
|-------------|------------------|
| A0 | Optical Fiber Sensor Output |
| A4 | MPU6050 SDA |
| A5 | MPU6050 SCL |
| D2 | LCD D7 |
| D3 | LCD D6 |
| D4 | LCD D5 |
| D5 | LCD D4 |
| D6 | Green LED |
| D7 | Yellow LED |
| D8 | Red LED |
| D9 | HC-SR04 TRIG |
| D10 | HC-SR04 ECHO |
| D11 | LCD Enable (EN) |
| D12 | LCD Register Select (RS) |
| D13 | DHT11 DATA |

---

## ESP8266 Connections

| ESP8266 Pin | Connection |
|-------------|------------|
| RX | Arduino TX (through voltage divider) |
| TX | Arduino RX |
| VCC | 3.3V |
| GND | GND |
| CH_PD / EN | 3.3V |
| GPIO0 | HIGH (Normal Operation) |

---

## Power Connections

| Device | Supply |
|---------|--------|
| MPU6050 | 5V |
| HC-SR04 | 5V |
| DHT11 | 5V |
| LCD | 5V |
| ESP8266 | 3.3V |

> **Important**
>
> - ESP8266 uses **3.3V logic**.
> - Always use a **voltage divider** between the Arduino TX pin and ESP8266 RX pin.
> - Connect all grounds together.
> - Use a **10kΩ pull-up resistor** on the DHT11 data line.
> - Use **220Ω resistors** with LEDs.

---

---

# Project Structure

```
SHM-Bridge-Monitoring/
│
├── README.md
├── LICENSE
├── .gitignore
│
├── src/
│   ├── main_shm_system/
│   │   └── main_shm_system.ino
│   │
│   └── esp8266_thingspeak/
│       └── esp8266_thingspeak.ino
│
├── tests/
│   ├── test_accelerometer/
│   │   └── test_accelerometer.ino
│   │
│   ├── test_displacement_ultrasonic/
│   │   └── test_displacement_ultrasonic.ino
│   │
│   ├── test_temperature/
│   │   └── test_temperature.ino
│   │
│   └── test_optical_fiber/
│       └── test_optical_fiber.ino
│
├── docs/
│   └── circuit_pinout.txt
│
├── images/
│
├── circuit/
│
└── datasheets/
```

---

# Installation & Setup

## 1. Clone the Repository

```bash
git clone https://github.com/shrawan-kumawat/SHM-Bridge-Monitoring.git

cd SHM-Bridge-Monitoring
```

---

## 2. Install Arduino IDE

Download the latest version of the Arduino IDE from:

https://www.arduino.cc/en/software

---

## 3. Install Required Boards

### Arduino Uno

Already included with the Arduino IDE.

### ESP8266

Open:

```
File → Preferences
```

Add the following URL under **Additional Boards Manager URLs**:

```
http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

Then open

```
Tools → Board → Boards Manager
```

Search for

```
ESP8266
```

and install the latest package.

---

## 4. Install Required Libraries

Install the following libraries from the Arduino Library Manager.

### Arduino Uno

- Wire

### ESP8266

- ESP8266WiFi
- ESP8266HTTPClient

---

## 5. Configure Wi-Fi

Open

```
src/esp8266_thingspeak/esp8266_thingspeak.ino
```

Replace

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

with your Wi-Fi credentials.

---

## 6. Configure ThingSpeak

Replace

```cpp
const String THINGSPEAK_API_KEY = "YOUR_WRITE_API_KEY";
```

with your Write API Key.

---

## 7. Upload Programs

### Arduino Uno

Upload

```
src/main_shm_system/main_shm_system.ino
```

### ESP8266

Upload

```
src/esp8266_thingspeak/esp8266_thingspeak.ino
```

---

## 8. Verify Operation

The system should

- Read all sensors
- Display values on the LCD
- Indicate bridge health using LEDs
- Upload data to ThingSpeak every 15–16 seconds

---

# Testing

Each sensor can be tested independently before running the complete SHM system.

| Test Program | Purpose |
|--------------|---------|
| `test_accelerometer.ino` | Verify MPU6050 acceleration and vibration measurements |
| `test_displacement_ultrasonic.ino` | Verify HC-SR04 distance and displacement calculations |
| `test_temperature.ino` | Verify DHT11 temperature and humidity readings |
| `test_optical_fiber.ino` | Verify optical fiber prototype response to deformation |

Running these standalone sketches simplifies debugging by validating each sensor individually before integrating the complete system.

---

# ThingSpeak Cloud Dashboard

The ESP8266 uploads processed sensor data to the ThingSpeak IoT platform approximately every **16 seconds** (compatible with the free-tier upload interval).

The dashboard stores and visualizes the following parameters:

| Field | Data |
|--------|------|
| Field 1 | Acceleration X |
| Field 2 | Acceleration Y |
| Field 3 | Acceleration Z |
| Field 4 | Vibration Magnitude |
| Field 5 | Displacement |
| Field 6 | Temperature |
| Field 7 | Optical Fiber Strain (Prototype) |
| Field 8 | Structural Health Status |

The dashboard enables:

- Real-time monitoring
- Historical data logging
- Trend visualization
- Remote access through the internet

> **Note:** Keep your ThingSpeak **Write API Key** private. Use placeholders in the source code instead of committing actual credentials.

---

# Experimental Results

The prototype successfully demonstrates a low-cost IoT-based Structural Health Monitoring system capable of:

- Monitoring bridge vibration using a tri-axis accelerometer.
- Measuring displacement using an ultrasonic sensor.
- Tracking environmental temperature and humidity.
- Demonstrating optical fiber-inspired strain sensing.
- Displaying local bridge status on a 16×2 LCD.
- Providing visual alerts using Green, Yellow, and Red LEDs.
- Uploading sensor data to the ThingSpeak cloud platform for remote monitoring.

The modular software architecture allows each sensing subsystem to be tested independently before full system integration.

---

# Future Enhancements

- Replace Arduino Uno + ESP8266 with ESP32.
- Integrate LoRa for long-range communication.
- Implement MQTT for real-time messaging.
- Deploy Grafana and InfluxDB for advanced visualization.
- Develop machine learning models for predictive maintenance.
- Add SMS and email notifications.
- Integrate solar power for autonomous deployment.
- Improve the optical fiber prototype using a commercial FBG interrogator.

---

# Contributing

Contributions, suggestions, and improvements are welcome.

If you find bugs or have ideas for new features, feel free to open an issue or submit a pull request.

---

# License

This project is licensed under the **MIT License**.

See the [LICENSE](LICENSE) file for details.

---

# Author

**Shrawan Kumawat**

B.Tech in Electronics and Communication Engineering


- GitHub: https://github.com/shrawan-kumawat
- LinkedIn: *www.linkedin.com/in/shrawankumawat*

---

⭐ If you found this project helpful, consider giving it a star on GitHub.
