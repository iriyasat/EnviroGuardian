# 🌍 EnviroGuardian 🛡️

![GitHub stars](https://img.shields.io/github/stars/iriyasat/EnviroGuardian?style=social)
<p align="center">
  <img src="EnviroGuardian%20Hardware%20setup.jpg" alt="EnviroGuardian Device" width="600"/>
</p>

> 🌿 Advanced IoT-based air quality monitoring system with cloud connectivity, real-time alerts, and multi-parameter sensing capabilities. Perfect for homes, classrooms and workspaces.

## ✨ Features

- 📊 Real-time monitoring of temperature, humidity, air quality, and carbon monoxide
- 🚦 Visual alerts through traffic light system (Green, Yellow, Red)
- 🔊 Audible alerts for dangerous conditions
- ☁️ Cloud connectivity through Arduino IoT Cloud
- 📱 Remote monitoring and notifications
- 🔄 Four dynamic display modes (Clock, Temperature/Humidity, Air Quality, System Info)
- 🎬 Beautiful startup animation and intuitive interface

## 🔧 Components

<table>
  <tr>
    <td><b>Controller</b></td>
    <td>ESP8266 NodeMCU</td>
  </tr>
  <tr>
    <td><b>Display</b></td>
    <td>1.3" OLED Display SH1106 (I²C, 128x64 pixels)</td>
  </tr>
  <tr>
    <td><b>Sensors</b></td>
    <td>
      • DHT22 (Temperature & humidity)<br>
      • MQ135 (Air quality)<br>
      • MQ9 (CO detection)
    </td>
  </tr>
  <tr>
    <td><b>Alert System</b></td>
    <td>
      • Green LED<br>
      • Yellow LED<br>
      • Red LED<br>
      • Active Buzzer (3-9V)
    </td>
  </tr>
  <tr>
    <td><b>Other</b></td>
    <td>
      • Resistors (220Ω for LEDs, 10kΩ & 20kΩ for voltage divider)<br>
      • Jumper Wires<br>
      • Breadboard
    </td>
  </tr>
</table>

## 📋 Connection Diagram

<p align="center">
  <img src="EnviroGuardian%20Circuit%20Diagram.png" alt="Circuit Diagram" width="700"/>
</p>

### Detailed Connections

<details>
<summary>Click to expand connection details</summary>

```
OLED Display:
- VCC → 3.3V on NodeMCU
- GND → GND on NodeMCU
- SCL → D1 (GPIO 5) on NodeMCU
- SDA → D2 (GPIO 4) on NodeMCU

DHT22 Sensor:
- VCC → 3.3V on NodeMCU
- GND → GND on NodeMCU
- DATA → D3 (GPIO 0) on NodeMCU

MQ135 Gas Sensor:
- VCC → 5V on NodeMCU
- GND → GND on NodeMCU
- AO → A0 on NodeMCU (Analog input)
- DO → D4 (GPIO 2) on NodeMCU (Digital output)

MQ9 Gas Sensor:
- VCC → 5V on NodeMCU
- GND → GND on NodeMCU
- AO → A0 on NodeMCU (Analog input)
- DO → D0 (GPIO 16) on NodeMCU (Digital output)

Alert System:
- Green LED → D5 (GPIO 14) on NodeMCU (with 220Ω resistor)
- Yellow LED → D6 (GPIO 12) on NodeMCU (with 220Ω resistor)
- Red LED → D7 (GPIO 13) on NodeMCU (with 220Ω resistor)
- Buzzer → D8 (GPIO 15) on NodeMCU
```
</details>

## 🚀 Getting Started

### Prerequisites
- Arduino IDE (1.8.x or higher)
- ESP8266 Board Manager
- Required libraries:
  - Arduino IoT Cloud
  - Adafruit GFX
  - Adafruit SH110X
  - DHT Sensor Library

### Installation
1. Clone this repository
   ```bash
   git clone https://github.com/iriyasat/EnviroGuardian.git
   ```
2. Open the project in Arduino IDE
3. Install the required libraries through the Library Manager
4. Configure your Arduino IoT Cloud credentials
5. Upload the sketch to your NodeMCU

## 🔌 Arduino IoT Cloud Setup

<details>
<summary>Click to see IoT Cloud configuration steps</summary>

1. Create a new Thing in Arduino IoT Cloud
2. Add the following properties:
   - `airQualityStatus` (String)
   - `coStatus` (String)
   - `currentDate` (String)
   - `currentTime` (String)
   - `dayOfWeek` (String)
   - `humidStatus` (String)
   - `ipAddress` (String)
   - `systemUptime` (String)
   - `tempStatus` (String)
   - `wifiStatus` (String)
   - `humidity` (Float)
   - `temperature` (Float)
   - `airQuality` (Int)
   - `coDetected` (Bool)
3. Create a dashboard with widgets for each property
</details>


## 🙏 Acknowledgements

- [Arduino](https://www.arduino.cc/) for the amazing IoT Cloud platform
- [Adafruit](https://www.adafruit.com/) for the excellent display and sensor libraries
- [ESP8266 Community](https://github.com/esp8266/Arduino) for the NodeMCU support

---

<p align="center">
  Made with ❤️ by [Ibrahim Hasan]
</p>
