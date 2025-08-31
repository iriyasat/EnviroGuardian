# 🌿 EnviroGuardian v1.0

**A sophisticated environmental monitoring system with advanced real-time web dashboard, built on ESP8266**

[![Version](https://img.shields.io/badge/Version-1.0-blue.svg)](#)
[![Platform](https://img.shields.io/badge/Platform-ESP8266-green.svg)](#)
[![Author](https://img.shields.io/badge/Author-Ibrahim%20Hasan-orange.svg)](#)

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [Wiring Diagram](#-wiring-diagram)
- [Installation](#-installation)
- [Configuration](#-configuration)
- [Advanced Web Dashboard](#-advanced-web-dashboard)
- [API Reference](#-api-reference)
- [Calibration](#-calibration)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)

## 🌟 Overview

EnviroGuardian v1.0 is a comprehensive environmental monitoring system that provides real-time air quality, temperature, humidity, and gas detection with an **advanced web interface featuring real-time charts**. Perfect for homes, offices, laboratories, or any space where air quality monitoring is important.

### Key Highlights

- **Professional-grade monitoring** with calibrated AQI calculations
- **Advanced responsive web dashboard** with real-time charts and animations
- **Real-time alerts** with visual (LED) and audio (buzzer) notifications
- **Three display modes** on OLED with auto-cycling every 10 seconds
- **Professional startup animation** with progress tracking
- **Data smoothing algorithms** for stable readings
- **Memory-optimized code** for stable long-term operation
- **Chart.js integration** for beautiful data visualization

## ✨ Features

### 📊 Environmental Monitoring
- **Temperature & Humidity** - Accurate DHT22 sensor readings
- **Air Quality Index (AQI)** - Calibrated calculations based on CO₂ levels  
- **CO₂ Concentration** - Real-time parts-per-million (ppm) readings
- **Gas Detection** - Harmful gas and CO detection with digital alerts

### 🌐 Connectivity & Interface
- **Advanced WiFi Web Dashboard** - Professional responsive interface with real-time charts
- **Live Data Streaming** - Real-time updates every 2 seconds with Chart.js visualization
- **RESTful API** - JSON endpoints for custom integrations
- **OLED Display** - Three auto-cycling information modes (10-second intervals)

### 🚨 Alert System
- **Traffic Light LEDs** - Green (Good), Yellow (Moderate), Red (Dangerous)
- **Audio Alerts** - Musical startup tones and emergency beeping
- **Smart Thresholds** - Configurable alert levels to prevent false alarms
- **Visual Indicators** - Clear status displays on all interfaces

### 🔧 Professional Features
- **Data Smoothing** - 10-sample moving average for stable readings
- **Memory Management** - Optimized for 24/7 operation
- **Auto-Reconnect** - Intelligent WiFi recovery system
- **System Monitoring** - Uptime, memory usage, and connection status
- **Real-time Charts** - Historical data visualization with smooth animations

## 🛠 Hardware Requirements

### Core Components
| Component | Model | Purpose | Price Range |
|-----------|-------|---------|-------------|
| **Microcontroller** | ESP8266 (NodeMCU/Wemos D1 Mini) | Main processing unit | $3-8 |
| **Display** | SH1106 OLED 128x64 (I2C) | Status display | $3-6 |
| **Temperature Sensor** | DHT22 | Temperature & humidity | $2-5 |
| **Air Quality Sensor** | MQ135 (5V operation) | CO₂ and air quality | $2-4 |
| **Gas Sensor** | MQ9 | Carbon monoxide detection | $2-4 |

### Visual & Audio Alerts
| Component | Specification | Purpose | Price Range |
|-----------|---------------|---------|-------------|
| **Traffic Light LED** | RGB LED or Traffic Light Module | Red/Yellow/Green status indicators | $1-3 |
| **Dual Active Buzzers** | 2x Active buzzer 5V (same pin) | Enhanced audio alerts | $2-4 |
| **Resistors** | 220Ω (for LED current limiting) | Component protection | $0.10 |

### Optional Components
- **Breadboard or PCB** - For permanent installation
- **Enclosure** - Professional housing
- **Power Supply** - 5V 2A USB adapter

## 🔌 Wiring Diagram

### ESP8266 Pin Connections

```
ESP8266 NodeMCU Pin Layout:
┌─────────────────────────────┐
│  [ ]3.3V  [ ]EN   [ ]D8    │
│  [ ]GND   [ ]A0   [ ]D7    │
│  [ ]D0    [ ]D1   [ ]D6    │
│  [ ]D1    [ ]D2   [ ]D5    │
│  [ ]D2    [ ]D3   [ ]GND   │
│  [ ]D3    [ ]D4   [ ]3.3V  │
│  [ ]D4    [ ]GND  [ ]VIN   │
└─────────────────────────────┘
```

### Complete Wiring Table

| ESP8266 Pin | Component | Wire Color | Notes |
|-------------|-----------|------------|-------|
| **VIN (5V)** | MQ135 VCC, MQ9 VCC | Red | 5V power for gas sensors |
| **3.3V** | OLED VCC, DHT22 VCC | Red | 3.3V power supply |
| **GND** | All GND connections | Black | Common ground |
| **D1 (GPIO5)** | OLED SCL | Yellow | I2C Clock |
| **D2 (GPIO4)** | OLED SDA | Green | I2C Data |
| **D5** | DHT22 Data | Blue | Temperature sensor |
| **A0** | MQ135 Analog | Orange | Air quality analog (5V) |
| **D7** | MQ135 Digital | Purple | Air quality digital |
| **D6** | MQ9 Digital | Brown | Gas sensor digital |
| **D0** | Traffic Light Green | Green | Green status (gets power from pin) |
| **D8** | Traffic Light Yellow | Yellow | Yellow status (gets power from pin) |
| **D3** | Traffic Light Red | Red | Red status (gets power from pin) |
| **D4** | Dual Buzzers + | White | 2x Audio buzzers (same pin) |

**Note:** Pin mapping matches your actual wiring: D0=Green, D3=Red, D8=Yellow

### Important: 5V Operation

**⚠️ Critical:** MQ135 and MQ9 sensors operate on 5V for proper sensitivity:
- Connect **MQ135 VCC → ESP8266 VIN (5V)**
- Connect **MQ9 VCC → ESP8266 VIN (5V)**  
- The ESP8266 A0 pin can safely read 0-3.3V from the 5V sensor

## 🚀 Installation

### 1. Arduino IDE Setup

1. **Install ESP8266 Board Package:**
   - File → Preferences
   - Add to Additional Board Manager URLs:
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Tools → Board → Boards Manager
   - Search "ESP8266" and install

2. **Install Required Libraries:**
   ```
   Tools → Manage Libraries → Search and Install:
   - Adafruit SH110X by Adafruit
   - DHT sensor library by Adafruit
   - ESP8266WiFi (included with board package)
   - ESP8266WebServer (included with board package)
   ```

### 2. Hardware Assembly

1. **Wire components** according to the wiring diagram above
2. **Double-check 5V connections** - MQ sensors need 5V for proper operation
3. **Test continuity** with a multimeter if available
4. **Secure connections** - loose wires cause intermittent issues

### 3. Software Configuration

1. **Download the code** from this repository
2. **Open** `EnviroGuardian.ino` in Arduino IDE
3. **Update WiFi credentials** in the configuration section:
   ```cpp
   const char* ssid = "Your_WiFi_Name";
   const char* password = "Your_WiFi_Password";
   ```
4. **Select board and port:**
   - Tools → Board → ESP8266 Boards → NodeMCU 1.0
   - Tools → Port → [Select your COM port]

### 4. Upload and Test

1. **Compile and upload** the code to ESP8266
2. **Open Serial Monitor** (115200 baud) to view startup process
3. **Note the IP address** displayed during WiFi connection
4. **Access web dashboard** at `http://[IP_ADDRESS]`

## ⚙️ Configuration

### WiFi Settings

Update these variables in the code:
```cpp
// WiFi Credentials - UPDATE THESE
const char* ssid = "Your_Network_Name";
const char* password = "Your_Network_Password";
```

### Alert Thresholds

Customize sensitivity in the configuration section:
```cpp
// Alert Thresholds (Adjust for your environment)
#define AIR_QUALITY_GOOD 100       // AQI 0-100 (Good)
#define AIR_QUALITY_MODERATE 200   // AQI 101-200 (Moderate)  
#define TEMP_MAX 40.0              // Maximum safe temperature (°C)
#define HUMIDITY_MAX 85.0          // Maximum safe humidity (%)
```

### Sensor Calibration (5V Operation)

Fine-tune for your specific sensors:
```cpp
// MQ135 Sensor Calibration Constants (5V Operation)
#define MQ135_R0 76.0              // Baseline resistance (adjust after testing)
#define MQ135_RL 10.0              // Load resistance (check sensor documentation)
#define VOLTAGE_REF 5.0            // 5V operation for proper sensitivity
```

## 🌐 Advanced Web Dashboard

### New Features in v1.0
- **🔥 Real-time Charts** with Chart.js integration
- **🎨 Advanced Animations** and smooth transitions
- **📊 Historical Data** visualization (last 20 data points)
- **🌈 Dynamic Progress Bars** with color-coded statuses
- **✨ Glassmorphism Design** with modern aesthetics
- **📱 Enhanced Mobile Experience** with responsive design

### Dashboard Sections

#### Real-time Environmental Trends
- **Interactive Chart** showing temperature, humidity, and AQI over time
- **Smooth Animations** with data point transitions
- **Color-coded Lines** for easy data distinction
- **Auto-scaling** based on data range

#### Enhanced Climate Monitor
- **Large Value Displays** with smooth counter animations
- **Progress Bars** showing current levels vs. safe ranges
- **Color-coded Indicators** (green/yellow/red)
- **Hover Effects** for interactive experience

#### Advanced Air Quality Display
- **Real-time AQI** with animated progress visualization
- **CO₂ Concentration** with 5V sensor calibration
- **Dynamic Color Changes** based on air quality levels
- **Professional Badges** indicating sensor types

#### Enhanced Gas Detection
- **Visual Alert System** with pulsing animations for dangers
- **Status Badges** showing sensor operational modes
- **Safety Indicators** with clear SAFE/DETECTED states
- **Emergency Styling** for critical alerts

#### Comprehensive System Information
- **Network Status** with connection indicators
- **Real-time Uptime** tracking
- **Memory Usage** monitoring
- **Version Information** and operational mode

### Accessing the Dashboard

1. **Connect ESP8266** to WiFi (check Serial Monitor for IP)
2. **Open web browser** on any device connected to same network
3. **Navigate to** `http://[ESP8266_IP_ADDRESS]`
4. **Enjoy the advanced interface** with real-time charts and animations

## 📡 API Reference

### Endpoints

#### GET `/`
Returns the advanced dashboard HTML interface with charts.

#### GET `/api/data`
Returns JSON with all current sensor readings and system status.

### Response Format

```json
{
  "temperature": 23.5,
  "humidity": 65,
  "aqi": 45,
  "aqiCategory": "GOOD",
  "co2": 650,
  "co": 0.0,
  "mq135_analog": 156,
  "gasDetected": false,
  "coDetected": false,
  "alertLevel": 0,
  "wifiConnected": true,
  "ipAddress": "192.168.1.100",
  "uptime": "2h 45m",
  "freeRam": 45632
}
```

## 🔧 Calibration

### Initial Setup (First 48 Hours)

1. **Burn-in period:** Let sensors run for 24-48 hours for stabilization
2. **5V Operation:** Ensure MQ sensors are connected to 5V for proper sensitivity
3. **Clean air baseline:** Place in well-ventilated area during initial setup
4. **Monitor readings:** Normal indoor values should be:
   - **Temperature:** 20-25°C
   - **Humidity:** 40-60%
   - **CO₂:** 400-800 ppm
   - **MQ135 analog:** 2-15 (clean air with 5V operation)

### 5V Sensor Calibration

The MQ135 and MQ9 sensors are designed for 5V operation:
- **Higher Sensitivity:** 5V provides better detection range
- **Accurate Readings:** Proper voltage ensures correct gas concentration calculations
- **Stable Operation:** 5V reduces noise and improves signal quality

## 🛠 Troubleshooting

### Common Issues

#### Sensor Reading Issues (5V Operation)
**Symptoms:** Low sensitivity, inaccurate readings
**Solutions:**
- Verify MQ135 and MQ9 are connected to **5V (VIN pin)**
- Check that analog reading comes from **A0 pin**
- Allow 15-20 minutes warm-up time for 5V operation
- Test with known gas sources (breath test for CO₂)

#### Web Dashboard Chart Issues
**Symptoms:** Charts not loading, JavaScript errors
**Solutions:**
- Ensure stable internet connection for Chart.js CDN
- Check browser console for JavaScript errors
- Clear browser cache and refresh page
- Verify ESP8266 has sufficient memory (>40KB free)

## 🤝 Contributing

We welcome contributions to improve EnviroGuardian! Here's how you can help:

### Development Setup

1. **Fork** this repository
2. **Create feature branch:** `git checkout -b feature/amazing-feature`
3. **Make changes** and test thoroughly
4. **Commit changes:** `git commit -m 'Add amazing feature'`
5. **Push to branch:** `git push origin feature/amazing-feature`
6. **Open Pull Request**

### Areas for Contribution

- **Additional Sensors:** Support for PM2.5, VOC sensors
- **Data Logging:** SD card or cloud storage integration
- **Mobile App:** Native smartphone applications
- **Advanced Charts:** More visualization options
- **Home Assistant:** MQTT integration
- **3D Enclosures:** Professional housing designs

## 📧 Contact & Support

- **GitHub Issues:** [Report bugs or request features](../../issues)
- **Documentation:** [Project Wiki](../../wiki)
- **Updates:** Watch this repository for latest improvements

---

### ⭐ If this project helped you, please give it a star!

**Made with ❤️ by Ibrahim Hasan**

*EnviroGuardian v1.0 - Advanced Environmental Monitoring for Everyone*

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [Wiring Diagram](#-wiring-diagram)
- [Installation](#-installation)
- [Configuration](#-configuration)
- [Web Dashboard](#-web-dashboard)
- [API Reference](#-api-reference)
- [Calibration](#-calibration)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)
- [License](#-license)

## 🌟 Overview

EnviroGuardian Professional is a comprehensive environmental monitoring system that provides real-time air quality, temperature, humidity, and gas detection with a beautiful web interface. Perfect for homes, offices, laboratories, or any space where air quality monitoring is important.

### Key Highlights

- **Professional-grade monitoring** with calibrated AQI calculations
- **Beautiful responsive web dashboard** accessible from any device
- **Real-time alerts** with visual (LED) and audio (buzzer) notifications
- **Three display modes** on OLED with auto-cycling
- **Professional startup animation** with progress tracking
- **Data smoothing algorithms** for stable readings
- **Memory-optimized code** for stable long-term operation

## ✨ Features

### 📊 Environmental Monitoring
- **Temperature & Humidity** - Accurate DHT22 sensor readings
- **Air Quality Index (AQI)** - Calibrated calculations based on CO₂ levels
- **CO₂ Concentration** - Real-time parts-per-million (ppm) readings
- **Gas Detection** - Harmful gas and CO detection with digital alerts

### 🌐 Connectivity & Interface
- **WiFi Web Dashboard** - Professional responsive interface
- **Real-time Updates** - Live data streaming every 2 seconds
- **RESTful API** - JSON endpoints for custom integrations
- **OLED Display** - Three auto-cycling information modes

### 🚨 Alert System
- **Traffic Light LEDs** - Green (Good), Yellow (Moderate), Red (Dangerous)
- **Audio Alerts** - Musical startup tones and emergency beeping
- **Smart Thresholds** - Configurable alert levels to prevent false alarms
- **Visual Indicators** - Clear status displays on all interfaces

### 🔧 Professional Features
- **Data Smoothing** - 10-sample moving average for stable readings
- **Memory Management** - Optimized for 24/7 operation
- **Auto-Reconnect** - Intelligent WiFi recovery system
- **System Monitoring** - Uptime, memory usage, and connection status

## 🛠 Hardware Requirements

### Core Components
| Component | Model | Purpose | Price Range |
|-----------|-------|---------|-------------|
| **Microcontroller** | ESP8266 (NodeMCU/Wemos D1 Mini) | Main processing unit | $3-8 |
| **Display** | SH1106 OLED 128x64 (I2C) | Status display | $3-6 |
| **Temperature Sensor** | DHT22 | Temperature & humidity | $2-5 |
| **Air Quality Sensor** | MQ135 | CO₂ and air quality | $2-4 |
| **Gas Sensor** | MQ9 | Carbon monoxide detection | $2-4 |

### Visual & Audio Alerts
| Component | Specification | Purpose | Price Range |
|-----------|---------------|---------|-------------|
| **LEDs** | 5mm Red, Yellow, Green | Status indicators | $0.50 |
| **Buzzer** | Active buzzer 5V | Audio alerts | $1-2 |
| **Resistors** | 220Ω (3x for LEDs) | Current limiting | $0.10 |

### Optional Components
- **Breadboard or PCB** - For permanent installation
- **Enclosure** - Professional housing
- **Power Supply** - 5V 2A USB adapter

## 🔌 Wiring Diagram

### ESP8266 Pin Connections

```
ESP8266 NodeMCU Pin Layout:
┌─────────────────────────────┐
│  [ ]3.3V  [ ]EN   [ ]D8    │
│  [ ]GND   [ ]A0   [ ]D7    │
│  [ ]D0    [ ]D1   [ ]D6    │
│  [ ]D1    [ ]D2   [ ]D5    │
│  [ ]D2    [ ]D3   [ ]GND   │
│  [ ]D3    [ ]D4   [ ]3.3V  │
│  [ ]D4    [ ]GND  [ ]VIN   │
└─────────────────────────────┘
```

### Complete Wiring Table

| ESP8266 Pin | Component | Wire Color | Notes |
|-------------|-----------|------------|-------|
| **3.3V** | OLED VCC, Sensors VCC | Red | Power supply |
| **GND** | All GND connections | Black | Common ground |
| **D1 (GPIO5)** | OLED SCL | Yellow | I2C Clock |
| **D2 (GPIO4)** | OLED SDA | Green | I2C Data |
| **D5** | DHT22 Data | Blue | Temperature sensor |
| **A0** | MQ135 Analog | Orange | Air quality analog |
| **D7** | MQ135 Digital | Purple | Air quality digital |
| **D6** | MQ9 Digital | Brown | Gas sensor digital |
| **D0** | Green LED + | Green | Good status |
| **D8** | Yellow LED + | Yellow | Moderate status |
| **D3** | Red LED + | Red | Alert status |
| **D4** | Buzzer + | White | Audio alerts |

### Traffic Light LED Configuration

**Choose ONE configuration that matches your hardware:**

#### Option A: RGB LED (Single LED, 4 pins)
```cpp
// Uncomment these lines in code:
#define USE_RGB_LED
#define LED_RED_PIN D0        // Red pin of RGB LED
#define LED_GREEN_PIN D8      // Green pin of RGB LED  
#define LED_BLUE_PIN D3       // Blue pin of RGB LED
```

**Wiring:**
```
RGB LED Common Cathode:
├── Red Pin → D0 → 220Ω resistor
├── Green Pin → D8 → 220Ω resistor
├── Blue Pin → D3 → 220Ω resistor
└── Common → GND
```

#### Option B: Traffic Light Module (Current Default)
```cpp
// These lines are active by default:
#define USE_TRAFFIC_MODULE
#define TRAFFIC_LIGHT_RED D0     // Red signal control
#define TRAFFIC_LIGHT_YELLOW D8  // Yellow signal control
#define TRAFFIC_LIGHT_GREEN D3   // Green signal control
```

**Wiring:**
```
Traffic Light Module:
├── Red Control → D0
├── Yellow Control → D8
├── Green Control → D3
├── VCC → 5V
└── GND → GND
```

#### Option C: Single Pin Control
```cpp
// Uncomment if you have single-pin traffic light:
#define USE_SINGLE_PIN
#define TRAFFIC_CONTROL_PIN D0
```

### Dual Buzzer System

**Enhanced Audio Setup:**
```
Dual Active Buzzers:
├── Buzzer 1 → D4 (positive), GND (negative)
├── Buzzer 2 → D4 (positive), GND (negative)
└── Benefits: Louder alerts, redundancy, better sound quality
```

**Note:** Both buzzers connected to the same pin for synchronized operation and enhanced volume.

### Sensor-Specific Wiring

#### MQ135 Air Quality Sensor
```
MQ135    →    ESP8266
VCC      →    5V
GND      →    GND
A0       →    A0 (analog reading)
D0       →    D7 (digital alert)
```

#### MQ9 Gas Sensor (Digital Only)
```
MQ9      →    ESP8266
VCC      →    5V
GND      →    GND
A0       →    [Not Connected]
D0       →    D6 (digital alert)
```

#### DHT22 Temperature Sensor
```
DHT22    →    ESP8266
VCC      →    3.3V
GND      →    GND
Data     →    D5
```

#### SH1106 OLED Display
```
OLED     →    ESP8266
VCC      →    3.3V
GND      →    GND
SCL      →    D1 (GPIO5)
SDA      →    D2 (GPIO4)
```

## 🚀 Installation

### 1. Arduino IDE Setup

1. **Install ESP8266 Board Package:**
   - File → Preferences
   - Add to Additional Board Manager URLs:
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Tools → Board → Boards Manager
   - Search "ESP8266" and install

2. **Install Required Libraries:**
   ```
   Tools → Manage Libraries → Search and Install:
   - Adafruit SH110X by Adafruit
   - DHT sensor library by Adafruit
   - ESP8266WiFi (included with board package)
   - ESP8266WebServer (included with board package)
   ```

### 2. Hardware Assembly

1. **Wire components** according to the wiring diagram above
2. **Double-check connections** - incorrect wiring can damage components
3. **Test continuity** with a multimeter if available
4. **Secure connections** - loose wires cause intermittent issues

### 3. Software Configuration

1. **Download the code** from this repository
2. **Open** `EnviroGuardian.ino` in Arduino IDE
3. **Update WiFi credentials** in the configuration section:
   ```cpp
   const char* ssid = "Your_WiFi_Name";
   const char* password = "Your_WiFi_Password";
   ```
4. **Select board and port:**
   - Tools → Board → ESP8266 Boards → NodeMCU 1.0
   - Tools → Port → [Select your COM port]

### 4. Upload and Test

1. **Compile and upload** the code to ESP8266
2. **Open Serial Monitor** (115200 baud) to view startup process
3. **Note the IP address** displayed during WiFi connection
4. **Access web dashboard** at `http://[IP_ADDRESS]`

## ⚙️ Configuration

### WiFi Settings

Update these variables in the code:
```cpp
// WiFi Credentials - UPDATE THESE
const char* ssid = "Your_Network_Name";
const char* password = "Your_Network_Password";
```

### Alert Thresholds

Customize sensitivity in the configuration section:
```cpp
// Alert Thresholds (Adjust for your environment)
#define AIR_QUALITY_GOOD 100       // AQI 0-100 (Good)
#define AIR_QUALITY_MODERATE 200   // AQI 101-200 (Moderate)  
#define TEMP_MAX 40.0              // Maximum safe temperature (°C)
#define HUMIDITY_MAX 85.0          // Maximum safe humidity (%)
```

### Sensor Calibration

Fine-tune for your specific sensors:
```cpp
// MQ135 Sensor Calibration Constants
#define MQ135_R0 76.0              // Baseline resistance (adjust after testing)
#define MQ135_RL 10.0              // Load resistance (check sensor documentation)
```

## 🌐 Web Dashboard

### Features
- **Real-time monitoring** with 2-second updates
- **Responsive design** - works on phones, tablets, computers
- **Professional UI** with gradient themes and smooth animations
- **Status indicators** with color-coded alerts
- **Progress bars** for visual data representation

### Accessing the Dashboard

1. **Connect ESP8266** to WiFi (check Serial Monitor for IP)
2. **Open web browser** on any device connected to same network
3. **Navigate to** `http://[ESP8266_IP_ADDRESS]`
4. **Bookmark** for easy access

### Dashboard Sections

#### Climate Monitor
- Temperature in Celsius with trend indicators
- Humidity percentage with comfort zones
- Real-time status badges

#### Air Quality
- AQI with color-coded progress bar
- CO₂ concentration in PPM
- Air quality categories (Good/Moderate/Unhealthy)

#### Gas Detection
- Gas sensor status with visual alerts
- CO detection with safety indicators
- Sensor type badges (MQ135/MQ9)

#### System Information
- WiFi connection status and IP address
- System uptime and memory usage
- Version information and mode indicators

## 📡 API Reference

### Endpoints

#### GET `/`
Returns the main dashboard HTML interface.

#### GET `/api/data`
Returns JSON with all current sensor readings and system status.

### Response Format

```json
{
  "temperature": 23.5,
  "humidity": 65,
  "aqi": 45,
  "aqiCategory": "GOOD",
  "co2": 650,
  "co": 0.0,
  "mq135_analog": 156,
  "gasDetected": false,
  "coDetected": false,
  "alertLevel": 0,
  "wifiConnected": true,
  "ipAddress": "192.168.1.100",
  "uptime": "2h 45m",
  "freeRam": 45632
}
```

### Integration Examples

#### Python
```python
import requests
import json

response = requests.get('http://192.168.1.100/api/data')
data = response.json()
print(f"Temperature: {data['temperature']}°C")
print(f"AQI: {data['aqi']} ({data['aqiCategory']})")
```

#### JavaScript
```javascript
fetch('http://192.168.1.100/api/data')
  .then(response => response.json())
  .then(data => {
    console.log('Temperature:', data.temperature + '°C');
    console.log('Air Quality:', data.aqi, data.aqiCategory);
  });
```

#### cURL
```bash
curl -X GET http://192.168.1.100/api/data
```

## 🔧 Calibration

### Initial Setup (First 48 Hours)

1. **Burn-in period:** Let sensors run for 24-48 hours for stabilization
2. **Clean air baseline:** Place in well-ventilated area during initial setup
3. **Monitor readings:** Normal indoor values should be:
   - **Temperature:** 20-25°C
   - **Humidity:** 40-60%
   - **CO₂:** 400-800 ppm
   - **MQ135 analog:** 2-15 (clean air)

### Fine-Tuning

#### MQ135 Air Quality Sensor

1. **Record clean air readings** over 24 hours
2. **Calculate average** of stable readings
3. **Update R0 value** in code:
   ```cpp
   #define MQ135_R0 [Your_Calculated_Value]
   ```

#### Testing Calibration

1. **Baseline test:** Normal readings in clean air
2. **Response test:** Breathe on sensor (should spike to 1000+ ppm)
3. **Recovery test:** Should return to baseline in 2-3 minutes
4. **Smoke test:** Light match nearby (should trigger gas alert)

### Expected Reading Ranges

| Condition | CO₂ (ppm) | AQI | MQ135 Raw | Status |
|-----------|-----------|-----|-----------|--------|
| **Outdoor Air** | 350-420 | 0-20 | 1-5 | Excellent |
| **Well Ventilated** | 400-600 | 20-40 | 5-10 | Good |
| **Normal Indoor** | 600-1000 | 40-80 | 10-20 | Acceptable |
| **Poor Ventilation** | 1000-1500 | 80-120 | 20-50 | Poor |
| **Stuffy Room** | 1500+ | 120+ | 50+ | Alert |

## 🛠 Troubleshooting

### Common Issues

#### WiFi Connection Problems
**Symptoms:** No web access, "WiFi: Offline" on display
**Solutions:**
- Check WiFi credentials in code
- Verify network is 2.4GHz (ESP8266 doesn't support 5GHz)
- Move closer to router during initial setup
- Check Serial Monitor for connection errors

#### Sensor Reading Issues
**Symptoms:** Constant 400 ppm CO₂, unchanging values
**Solutions:**
- Check MQ135 wiring (especially analog connection to A0)
- Verify 5V power supply to sensors
- Allow 10-15 minutes warm-up time
- Test with breath (should cause CO₂ spike)

#### Display Problems
**Symptoms:** Blank OLED, garbled display
**Solutions:**
- Verify I2C connections (SDA to D2, SCL to D1)
- Check OLED I2C address (should be 0x3C)
- Ensure 3.3V power to display
- Try different I2C address in code if needed

#### Memory Issues
**Symptoms:** Random resets, web server crashes
**Solutions:**
- Use latest ESP8266 Arduino core
- Monitor free heap in Serial output
- Reduce web update frequency if needed
- Check for memory leaks in custom code

#### Constant Alerts
**Symptoms:** Always shows yellow/red LED, frequent buzzer
**Solutions:**
- Increase alert thresholds in configuration
- Check for proper sensor calibration
- Verify clean air baseline readings
- Allow sensors to stabilize after power-on

### Diagnostic Tools

#### Serial Monitor Output
Enable verbose output to diagnose issues:
```cpp
Serial.printf("Raw MQ135: %d, CO2: %.0f ppm, AQI: %.0f\n", 
              mq135_analog, co2_ppm, calculated_aqi);
```

#### Web API Testing
Test API endpoints directly:
```bash
# Test basic connectivity
ping 192.168.1.100

# Test web server
curl -I http://192.168.1.100

# Test API endpoint
curl http://192.168.1.100/api/data
```

### Hardware Verification

#### Voltage Testing
- **ESP8266 VIN:** 5V ±0.2V
- **ESP8266 3.3V:** 3.3V ±0.1V
- **Sensor VCC:** 5V ±0.2V
- **I2C Lines:** 3.3V when idle

#### Continuity Testing
- Verify all ground connections
- Check power rail continuity
- Test I2C line connections
- Confirm analog pin connection

## 🤝 Contributing

We welcome contributions to improve EnviroGuardian! Here's how you can help:

### Development Setup

1. **Fork** this repository
2. **Create feature branch:** `git checkout -b feature/amazing-feature`
3. **Make changes** and test thoroughly
4. **Commit changes:** `git commit -m 'Add amazing feature'`
5. **Push to branch:** `git push origin feature/amazing-feature`
6. **Open Pull Request**

### Contribution Guidelines

- **Code Style:** Follow existing formatting and commenting patterns
- **Testing:** Test on actual hardware before submitting
- **Documentation:** Update README.md for new features
- **Backwards Compatibility:** Maintain compatibility with existing setups

### Areas for Contribution

- **Additional Sensors:** Support for more environmental sensors
- **Data Logging:** SD card or cloud storage integration
- **Mobile App:** Native smartphone applications
- **Enclosure Designs:** 3D printable cases
- **Advanced Analytics:** Historical data trends and analysis
- **Protocol Support:** MQTT, InfluxDB, Home Assistant integration

## 🙏 Acknowledgments

- **Adafruit** - For excellent sensor libraries
- **ESP8266 Community** - For comprehensive documentation
- **Arduino Team** - For the amazing development platform
- **Open Source Contributors** - For inspiration and code improvements

## 📧 Contact & Support

- **GitHub Issues:** [Report bugs or request features](../../issues)
- **Documentation:** [Project Wiki](../../wiki)
- **Updates:** Watch this repository for latest improvements

---

### ⭐ If this project helped you, please give it a star!

**Made with ❤️ by Group: 7 (Section:2) **

*EnviroGuardian V1.0 - Environmental Monitoring for Everyone*
