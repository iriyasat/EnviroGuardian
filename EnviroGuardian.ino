/*
 * EnviroGuardian v1.0
 * 
 * Advanced Environmental Monitoring System with Real-time Web Dashboard
 * 
 * Features:
 * - Real-time temperature and humidity monitoring (DHT22)
 * - Air quality monitoring with calibrated AQI calculation (MQ135 - 5V)
 * - Carbon monoxide detection (MQ9)
 * - Advanced web dashboard
 * - Traffic light LED status system (3 LEDs)
 * - Dual active buzzer audio alerts
 * - OLED display with 3 cycling modes (10-second intervals)
 * 
 * Hardware:
 * - ESP8266 (NodeMCU/Wemos D1 Mini)
 * - SH1106 OLED Display (128x64)
 * - DHT22 Temperature & Humidity Sensor
 * - MQ135 Air Quality Sensor (5V operation)
 * - MQ9 Gas Sensor
 * - Traffic Light Module (Green, Yellow, Red LEDs)
 * - 2x Active Buzzers
 * 
 * Author: Ibrahim Hasan
 * Version: 1.0
 * Date: 2025
 */

#include <Wire.h>
#include <Adafruit_SH110X.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// ========================================
// CONFIGURATION
// ========================================

// WiFi Credentials - UPDATE THESE
const char* ssid = "Nurul Haq";
const char* password = "114819690";

// Web Server
ESP8266WebServer server(80);

// Hardware Objects
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

// ========================================
// HARDWARE PIN DEFINITIONS
// ========================================

// DHT22 Temperature & Humidity Sensor
#define DHTPIN D5           // GPIO16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// MQ135 Air Quality Sensor (5V operation)
#define MQ135_AO A0         // Analog output to ESP8266 A0
#define MQ135_DO D7         // Digital output (GPIO13)

// MQ9 Gas Sensor
#define MQ9_DO D6           // Digital output (GPIO12)

// Traffic Light LED System (Your exact wiring)
#define TRAFFIC_LIGHT_GREEN D0    // Green LED control (GPIO16)
#define TRAFFIC_LIGHT_YELLOW D8   // Yellow LED control (GPIO15)
#define TRAFFIC_LIGHT_RED D3      // Red LED control (GPIO0)

// Dual Active Buzzers System
#define BUZZER_PIN D4       // GPIO2 - Both buzzers connected for enhanced audio

// ========================================
// SENSOR CALIBRATION & THRESHOLDS
// ========================================

// Alert Thresholds (Adjusted for reduced sensitivity)
#define AIR_QUALITY_GOOD 100       // AQI 0-100 (Good)
#define AIR_QUALITY_MODERATE 200   // AQI 101-200 (Moderate)
#define TEMP_MAX 40.0              // Maximum safe temperature (°C)
#define HUMIDITY_MAX 85.0          // Maximum safe humidity (%)

// MQ135 Sensor Calibration Constants (5V Operation)
#define MQ135_R0 76.0              // Baseline resistance for clean air (5V calibrated)
#define MQ135_RL 10.0              // Load resistance (10kΩ typical for MQ135)
#define VOLTAGE_REF 5.0            // MQ135 operates on 5V for proper sensitivity
#define ADC_MAX 1023.0             // 10-bit ADC maximum value

// Sensor Reading Bounds
#define MIN_ANALOG_READING 1       // Minimum valid analog reading
#define MAX_ANALOG_READING 1000    // Maximum expected analog reading

// Data Smoothing Configuration
#define SMOOTH_SAMPLES 10          // Number of samples for moving average

// ========================================
// SYSTEM ENUMERATIONS & DATA STRUCTURES
// ========================================

// Display Modes - Auto-cycling every 10 seconds
enum DisplayMode {
  MODE_STARTUP = 0,         // Startup animation
  MODE_TEMP_HUMIDITY = 1,   // Temperature & humidity display
  MODE_AIR_QUALITY = 2,     // Air quality & gas detection
  MODE_SYSTEM_INFO = 3      // System status & network info
};

// Alert Levels - Controls LEDs and audio
enum AlertLevel {
  GOOD = 0,                 // Green LED - All normal
  MODERATE = 1,             // Yellow LED - Caution
  DANGEROUS = 2             // Red LED + Audio - Alert
};

// ========================================
// GLOBAL VARIABLES
// ========================================

// Data Smoothing Arrays
float mq135_readings[SMOOTH_SAMPLES] = {0};
float temp_readings[SMOOTH_SAMPLES] = {0};
float humidity_readings[SMOOTH_SAMPLES] = {0};
int reading_index = 0;
bool arrays_filled = false;

// System State Variables
DisplayMode currentMode = MODE_STARTUP;
AlertLevel currentAlert = GOOD;
unsigned long lastModeSwitch = 0;
unsigned long lastAlertTime = 0;
unsigned long startupTime = 0;
bool buzzerState = false;
bool systemReady = false;
int animationFrame = 0;

// Sensor Data - Raw Readings
float temperature = 0;
float humidity = 0;
int mq135_analog = 0;
int mq135_digital = 0;
int mq9_digital = 0;

// Sensor Data - Processed & Smoothed
float mq135_smoothed = 0;
float temp_smoothed = 0;
float humidity_smoothed = 0;
float calculated_aqi = 0;
float co2_ppm = 0;
float co_ppm = 0;

// Startup Animation Variables
int progressBar = 0;
String loadingText = "Initializing";
bool startupComplete = false;

// ========================================
// SENSOR CALIBRATION FUNCTIONS
// ========================================

// Data Smoothing - Moving Average Filter
float smoothValue(float *values, int size, float newValue) {
  // Shift array and add new value
  for (int i = size - 1; i > 0; i--) {
    values[i] = values[i - 1];
  }
  values[0] = newValue;
  
  // Calculate average
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += values[i];
  }
  return sum / size;
}

// Improved MQ135 calibration functions for 5V operation
float calculateRs(int analogValue) {
  // Ensure we have a valid reading
  if (analogValue < MIN_ANALOG_READING) {
    analogValue = MIN_ANALOG_READING;
  }
  
  // Convert analog reading to voltage (5V operation)
  float voltage = ((float)analogValue / ADC_MAX) * VOLTAGE_REF;
  
  // Avoid division by zero
  if (voltage >= VOLTAGE_REF) {
    voltage = VOLTAGE_REF - 0.1;
  }
  
  // Calculate sensor resistance using voltage divider
  float rs = ((VOLTAGE_REF - voltage) / voltage) * MQ135_RL;
  
  // Ensure reasonable resistance values
  rs = constrain(rs, 0.1, 1000.0);
  
  return rs;
}

float calculateCO2ppm(float rs) {
  // Improved MQ135 CO2 curve for 5V operation
  float ratio = rs / MQ135_R0;
  
  // Handle edge cases
  if (ratio <= 0.01) ratio = 0.01;
  if (ratio >= 10.0) ratio = 10.0;
  
  // Modified CO2 calculation for better accuracy with 5V readings
  float ppm = 116.6020682 * pow(ratio, -2.769034857);
  
  // For very low analog readings, use direct mapping
  if (mq135_analog <= 10) {
    ppm = map(mq135_analog, 1, 10, 800, 500);
  }
  
  // Ensure reasonable CO2 range
  ppm = constrain(ppm, 350, 5000);
  
  return ppm;
}

float mapToAQI(float co2_ppm) {
  // Improved CO2 to AQI mapping
  if (co2_ppm <= 400) return 0;                                        // Excellent
  else if (co2_ppm <= 600) return map(co2_ppm, 400, 600, 1, 50);      // Good  
  else if (co2_ppm <= 1000) return map(co2_ppm, 600, 1000, 51, 100);  // Moderate
  else if (co2_ppm <= 1500) return map(co2_ppm, 1000, 1500, 101, 150); // Unhealthy for Sensitive
  else if (co2_ppm <= 2000) return map(co2_ppm, 1500, 2000, 151, 200); // Unhealthy
  else if (co2_ppm <= 5000) return map(co2_ppm, 2000, 5000, 201, 300); // Very Unhealthy
  else return 301; // Hazardous
}

String getAQICategory(float aqi) {
  if (aqi <= 50) return "GOOD";
  else if (aqi <= 100) return "MODERATE";
  else if (aqi <= 150) return "UNHEALTHY";
  else if (aqi <= 200) return "VERY BAD";
  else if (aqi <= 300) return "HAZARDOUS";
  else return "EXTREME";
}

// ========================================
// AUDIO SYSTEM FUNCTIONS
// ========================================

// Professional Startup Audio Sequence
void playStartupTone() {
  // Professional startup sequence tones
  tone(BUZZER_PIN, 523, 200); // C5
  delay(250);
  tone(BUZZER_PIN, 659, 200); // E5
  delay(250);
  tone(BUZZER_PIN, 784, 300); // G5
  delay(400);
  noTone(BUZZER_PIN);
}

void playAlertTone(AlertLevel level) {
  switch(level) {
    case GOOD:
      tone(BUZZER_PIN, 880, 100); // A5 - brief good tone
      delay(120);
      noTone(BUZZER_PIN);
      break;
    case MODERATE:
      tone(BUZZER_PIN, 440, 200); // A4 - warning tone
      delay(250);
      noTone(BUZZER_PIN);
      break;
    case DANGEROUS:
      // Urgent beeping pattern
      if (millis() - lastAlertTime > 300) {
        tone(BUZZER_PIN, 1000, 150);
        lastAlertTime = millis();
      }
      break;
  }
}

// ========================================
// NETWORK & CONNECTIVITY FUNCTIONS
// ========================================

// WiFi Connection with Auto-Reconnect
void connectWiFi() {
  // Only try to reconnect if actually disconnected
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 15) {
    delay(500);
    Serial.print(".");
    attempt++;
    
    // Update progress during WiFi connection
    if (!startupComplete) {
      progressBar = map(attempt, 0, 15, 50, 80);
      updateStartupDisplay();
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed - continuing offline");
  }
}

// ========================================
// OLED DISPLAY FUNCTIONS
// ========================================

// Progress Bar Drawing for Startup Animation
void drawProgressBar(int x, int y, int width, int height, int progress) {
  // Draw border
  display.drawRect(x, y, width, height, SH110X_WHITE);
  
  // Fill progress
  int fillWidth = map(progress, 0, 100, 0, width - 2);
  if (fillWidth > 0) {
    display.fillRect(x + 1, y + 1, fillWidth, height - 2, SH110X_WHITE);
  }
}

void drawAnimatedDots(int x, int y) {
  int numDots = (millis() / 500) % 4;
  display.setCursor(x, y);
  for (int i = 0; i < numDots; i++) {
    display.print(".");
  }
  // Clear remaining space to avoid artifacts
  for (int i = numDots; i < 3; i++) {
    display.print(" ");
  }
}

void updateStartupDisplay() {
  display.clearDisplay();
  
  // Title with animation
  display.setTextSize(1);
  display.setCursor(20, 5);
  display.print("EnviroGuardian");
  
  // Subtitle
  display.setCursor(35, 15);
  display.print("Professional");
  
  // Progress bar
  drawProgressBar(10, 28, 108, 8, progressBar);
  
  // Loading text with animated dots
  display.setCursor(20, 42);
  display.print(loadingText);
  drawAnimatedDots(20 + loadingText.length() * 6, 42);
  
  // Version info
  display.setCursor(55, 54);
  display.print("v1.0");
  
  display.display();
}

void displayTempHumidity() {
  display.clearDisplay();
  
  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("CLIMATE");
  
  // Status indicator
  display.setCursor(95, 0);
  switch(currentAlert) {
    case GOOD: display.print("OK"); break;
    case MODERATE: display.print("WARN"); break;
    case DANGEROUS: display.print("CRIT"); break;
  }
  
  // Separator line
  display.drawLine(0, 10, 127, 10, SH110X_WHITE);
  
  // Temperature section
  display.setCursor(0, 16);
  display.print("Temperature:");
  
  display.setCursor(0, 26);
  if (!isnan(temp_smoothed)) {
    display.setTextSize(1);
    display.printf("%.1f C", temp_smoothed);
  } else {
    display.print("ERROR");
  }
  
  // Humidity section
  display.setCursor(0, 40);
  display.print("Humidity:");
  
  display.setCursor(0, 50);
  if (!isnan(humidity_smoothed)) {
    display.setTextSize(1);
    display.printf("%.0f%%", humidity_smoothed);
  } else {
    display.print("ERROR");
  }
  
  display.display();
}

void displayAirQuality() {
  display.clearDisplay();
  
  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("AIR QUALITY");
  
  // Status indicator with AQI category
  display.setCursor(75, 0);
  display.print(getAQICategory(calculated_aqi));
  
  // Separator line
  display.drawLine(0, 10, 127, 10, SH110X_WHITE);
  
  // AQI Reading (from MQ135 5V operation)
  display.setCursor(0, 16);
  display.print("AQI:");
  display.setCursor(35, 16);
  display.printf("%.0f", calculated_aqi);
  
  // CO2 Reading (from MQ135)
  display.setCursor(70, 16);
  display.printf("%.0fppm", co2_ppm);
  
  // Show sensor readings
  display.setCursor(0, 28);
  display.printf("MQ135: %d", mq135_analog);
  
  display.setCursor(70, 28);
  display.printf("MQ9: %d", mq9_digital == LOW ? 1 : 0);
  
  // Gas detection alerts
  display.setCursor(0, 42);
  display.printf("Gas: %s", mq135_digital == LOW ? "YES" : "NO");
  
  display.setCursor(0, 52);
  display.printf("CO:  %s", mq9_digital == LOW ? "YES" : "NO");
  
  display.display();
}

void displaySystemInfo() {
  display.clearDisplay();
  
  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("SYSTEM INFO");
  
  // Separator line
  display.drawLine(0, 10, 127, 10, SH110X_WHITE);
  
  // WiFi Status
  display.setCursor(0, 18);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi: Connected");
  } else {
    display.print("WiFi: Offline");
  }
  
  // IP Address (if connected)
  if (WiFi.status() == WL_CONNECTED) {
    display.setCursor(0, 30);
    String ip = WiFi.localIP().toString();
    if (ip.length() > 21) {
      display.print(ip.substring(0, 21));
    } else {
      display.print(ip);
    }
  }
  
  // Uptime with larger spacing
  display.setCursor(0, 45);
  unsigned long uptime = (millis() - startupTime) / 1000;
  if (uptime < 3600) { // Less than 1 hour
    display.printf("Uptime: %lum %lus", uptime / 60, uptime % 60);
  } else { // 1 hour or more
    display.printf("Uptime: %luh %lum", uptime / 3600, (uptime % 3600) / 60);
  }
  
  display.display();
}

// ========================================
// ALERT & LED CONTROL FUNCTIONS
// ========================================

// Traffic Light LED Control (Your exact pin mapping)
void setTrafficLight(AlertLevel level) {
  // Turn off all LEDs first
  digitalWrite(TRAFFIC_LIGHT_GREEN, LOW);
  digitalWrite(TRAFFIC_LIGHT_YELLOW, LOW);
  digitalWrite(TRAFFIC_LIGHT_RED, LOW);
  
  switch(level) {
    case GOOD:
      digitalWrite(TRAFFIC_LIGHT_GREEN, HIGH);  // D0
      break;
    case MODERATE:
      digitalWrite(TRAFFIC_LIGHT_YELLOW, HIGH); // D8
      break;
    case DANGEROUS:
      digitalWrite(TRAFFIC_LIGHT_RED, HIGH);    // D3
      break;
  }
}

void handleAlerts() {
  AlertLevel newAlert = GOOD;
  
  // Determine alert level using calibrated AQI and sensors
  if (mq135_digital == LOW || mq9_digital == LOW || 
      temp_smoothed > TEMP_MAX || humidity_smoothed > HUMIDITY_MAX) {
    newAlert = DANGEROUS;
  } else if (calculated_aqi > AIR_QUALITY_MODERATE) {
    newAlert = DANGEROUS;
  } else if (calculated_aqi > AIR_QUALITY_GOOD) {
    newAlert = MODERATE;
  }
  
  // Handle alert level changes
  if (newAlert != currentAlert) {
    currentAlert = newAlert;
    setTrafficLight(currentAlert);
    playAlertTone(currentAlert);
  }
  
  // Handle continuous dangerous alert
  if (currentAlert == DANGEROUS) {
    playAlertTone(DANGEROUS);
  }
}

// ========================================
// SENSOR READING & PROCESSING FUNCTIONS
// ========================================

// Main Sensor Reading Function
void readSensors() {
  // Read raw sensor data
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // MQ135 Analog Handling with disconnection detection
  mq135_analog = analogRead(MQ135_AO);
  
  // Check if sensor is disconnected (reading below noise floor)
  bool mq135_disconnected = (mq135_analog < 10);
  
  if(mq135_disconnected) {
    mq135_analog = 0;  // Force to 0 for "disconnected" state
    mq135_digital = HIGH;  // Override digital reading
  } else {
    mq135_digital = digitalRead(MQ135_DO);
  }

  // MQ9 Handling with disconnection detection
  if(mq135_disconnected) {  // If MQ135 is disconnected, assume MQ9 is too
    mq9_digital = HIGH;     // Override digital reading
    co_ppm = 0.0;           // Force CO to 0
  } else {
    mq9_digital = digitalRead(MQ9_DO);
    co_ppm = mq9_digital == LOW ? 100.0 : 0.0;
  }

  // Apply smoothing to environmental data
  if (!isnan(temperature)) {
    temp_smoothed = smoothValue(temp_readings, SMOOTH_SAMPLES, temperature);
  }
  if (!isnan(humidity)) {
    humidity_smoothed = smoothValue(humidity_readings, SMOOTH_SAMPLES, humidity);
  }

  // Process MQ135 data only if connected
  if(!mq135_disconnected) {
    mq135_smoothed = smoothValue(mq135_readings, SMOOTH_SAMPLES, (float)mq135_analog);
    float rs_135 = calculateRs((int)mq135_smoothed);
    co2_ppm = calculateCO2ppm(rs_135);
    calculated_aqi = mapToAQI(co2_ppm);
  } else {
    // Safe defaults when disconnected
    co2_ppm = 400.0;  // Normal atmospheric CO2 level
    calculated_aqi = 0.0;
    mq135_smoothed = 0.0;
  }

  // Update sample index
  reading_index = (reading_index + 1) % SMOOTH_SAMPLES;
  if(reading_index == 0) arrays_filled = true;
}

// ========================================
// WEB SERVER & API FUNCTIONS
// ========================================

// Simplified Web Dashboard without Charts
const char* dashboard_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>EnviroGuardian v1.0 Dashboard</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        :root {
            --primary-gradient: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            --secondary-gradient: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            --success-color: #4CAF50;
            --warning-color: #FF9800;
            --danger-color: #F44336;
            --card-bg: rgba(255, 255, 255, 0.95);
            --glass-bg: rgba(255, 255, 255, 0.1);
            --text-primary: #2c3e50;
            --text-secondary: #7f8c8d;
            --shadow: 0 10px 30px rgba(0,0,0,0.2);
            --border-radius: 20px;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: var(--primary-gradient);
            min-height: 100vh;
            padding: 20px;
            overflow-x: hidden;
        }
        
        .container {
            max-width: 1400px;
            margin: 0 auto;
            animation: fadeInUp 0.8s ease-out;
        }
        
        @keyframes fadeInUp {
            from {
                opacity: 0;
                transform: translateY(30px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }
        
        @keyframes pulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.05); }
        }
        
        .header {
            text-align: center;
            color: white;
            margin-bottom: 40px;
            position: relative;
        }
        
        .header::before {
            content: '';
            position: absolute;
            top: -20px;
            left: 50%;
            transform: translateX(-50%);
            width: 100px;
            height: 4px;
            background: var(--secondary-gradient);
            border-radius: 2px;
        }
        
        .header h1 {
            font-size: 3rem;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
            background: linear-gradient(45deg, #fff, #f0f0f0);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }
        
        .header .subtitle {
            font-size: 1.2rem;
            opacity: 0.9;
            font-weight: 300;
        }
        
        .header .version {
            display: inline-block;
            background: var(--glass-bg);
            padding: 5px 15px;
            border-radius: 20px;
            margin-top: 10px;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255,255,255,0.2);
        }
        
        .status-banner {
            background: var(--glass-bg);
            border-radius: var(--border-radius);
            padding: 20px;
            margin-bottom: 40px;
            text-align: center;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255,255,255,0.2);
            position: relative;
            overflow: hidden;
        }
        
        .status-content {
            position: relative;
            z-index: 2;
        }
        
        .status-indicator {
            display: inline-block;
            width: 15px;
            height: 15px;
            border-radius: 50%;
            margin-right: 15px;
            box-shadow: 0 0 20px currentColor;
            animation: pulse 2s infinite;
        }
        
        .status-good { background-color: var(--success-color); }
        .status-moderate { background-color: var(--warning-color); }
        .status-dangerous { background-color: var(--danger-color); }
        
        .status-text {
            color: white;
            font-size: 1.4rem;
            font-weight: 600;
            text-shadow: 1px 1px 2px rgba(0,0,0,0.3);
        }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 30px;
            margin-bottom: 40px;
        }
        
        .card {
            background: var(--card-bg);
            border-radius: var(--border-radius);
            padding: 30px;
            box-shadow: var(--shadow);
            transition: all 0.3s ease;
            position: relative;
            overflow: hidden;
            border: 1px solid rgba(255,255,255,0.3);
        }
        
        .card:hover {
            transform: translateY(-10px) scale(1.02);
            box-shadow: 0 20px 40px rgba(0,0,0,0.3);
        }
        
        .card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 4px;
            background: var(--secondary-gradient);
        }
        
        .card-header {
            display: flex;
            align-items: center;
            margin-bottom: 25px;
        }
        
        .card-icon {
            font-size: 2.5rem;
            margin-right: 20px;
            padding: 15px;
            border-radius: 15px;
            background: var(--glass-bg);
            backdrop-filter: blur(10px);
        }
        
        .card-title {
            font-size: 1.4rem;
            font-weight: 700;
            color: var(--text-primary);
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .metric-value {
            font-size: 3.5rem;
            font-weight: 800;
            color: var(--text-primary);
            margin-bottom: 15px;
            text-shadow: 1px 1px 2px rgba(0,0,0,0.1);
            transition: all 0.3s ease;
        }
        
        .metric-unit {
            font-size: 1.4rem;
            color: var(--text-secondary);
            margin-left: 10px;
            font-weight: 400;
        }
        
        .metric-label {
            font-size: 1rem;
            color: var(--text-secondary);
            text-transform: uppercase;
            letter-spacing: 2px;
            font-weight: 500;
        }
        
        .progress-container {
            margin-top: 20px;
            position: relative;
        }
        
        .progress-bar {
            width: 100%;
            height: 12px;
            background: linear-gradient(90deg, #ecf0f1, #bdc3c7);
            border-radius: 6px;
            overflow: hidden;
            box-shadow: inset 0 2px 4px rgba(0,0,0,0.1);
        }
        
        .progress-fill {
            height: 100%;
            border-radius: 6px;
            transition: all 0.8s cubic-bezier(0.4, 0, 0.2, 1);
        }
        
        .aqi-excellent { background: linear-gradient(45deg, #4CAF50, #66BB6A); }
        .aqi-good { background: linear-gradient(45deg, #8BC34A, #9CCC65); }
        .aqi-moderate { background: linear-gradient(45deg, #FF9800, #FFB74D); }
        .aqi-unhealthy { background: linear-gradient(45deg, #F44336, #EF5350); }
        
        .gas-alert {
            padding: 15px 20px;
            border-radius: 12px;
            margin: 15px 0;
            font-weight: 600;
            display: flex;
            align-items: center;
            transition: all 0.3s ease;
        }
        
        .alert-safe {
            background: linear-gradient(135deg, #d4edda, #c3e6cb);
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        
        .alert-danger {
            background: linear-gradient(135deg, #f8d7da, #f5c6cb);
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        
        .badge {
            display: inline-block;
            padding: 6px 12px;
            border-radius: 20px;
            font-size: 0.8rem;
            font-weight: 600;
            background: var(--secondary-gradient);
            color: white;
            margin-left: 15px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .system-info {
            line-height: 2;
            font-size: 1.1rem;
        }
        
        .system-info strong {
            color: var(--text-primary);
            font-weight: 600;
        }
        
        .last-update {
            text-align: center;
            color: rgba(255,255,255,0.8);
            margin-top: 40px;
            font-size: 1rem;
            padding: 15px;
            background: var(--glass-bg);
            border-radius: var(--border-radius);
            backdrop-filter: blur(10px);
        }
        
        .pulse-dot {
            display: inline-block;
            width: 8px;
            height: 8px;
            background: #4CAF50;
            border-radius: 50%;
            margin-right: 10px;
            animation: pulse 1.5s infinite;
        }
        
        @media (max-width: 768px) {
            .header h1 { font-size: 2.5rem; }
            .metric-value { font-size: 3rem; }
            .card { padding: 25px; }
            .grid { grid-template-columns: 1fr; gap: 20px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌿 EnviroGuardian</h1>
            <p class="subtitle">Advanced Environmental Monitoring System</p>
            <div class="version">v1.0 by Ibrahim Hasan</div>
        </div>
        
        <div class="status-banner">
            <div class="status-content">
                <span class="status-indicator status-good" id="statusIndicator"></span>
                <span class="status-text" id="statusText">System Initializing...</span>
            </div>
        </div>

        <div class="grid">
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">🌡️</div>
                    <div class="card-title">Temperature</div>
                </div>
                <div class="metric-value" id="temperature">--<span class="metric-unit">°C</span></div>
                <div class="metric-label">Current Temperature</div>
                <div class="progress-container">
                    <div class="progress-bar">
                        <div class="progress-fill aqi-good" id="tempProgress" style="width: 0%"></div>
                    </div>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">💧</div>
                    <div class="card-title">Humidity</div>
                </div>
                <div class="metric-value" id="humidity">--<span class="metric-unit">%</span></div>
                <div class="metric-label">Relative Humidity</div>
                <div class="progress-container">
                    <div class="progress-bar">
                        <div class="progress-fill aqi-good" id="humidityProgress" style="width: 0%"></div>
                    </div>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">🌬️</div>
                    <div class="card-title">Air Quality Index</div>
                </div>
                <div class="metric-value" id="aqi">--</div>
                <div class="metric-label" id="aqiCategory">CALCULATING...</div>
                <div class="progress-container">
                    <div class="progress-bar">
                        <div class="progress-fill aqi-good" id="aqiProgress" style="width: 0%"></div>
                    </div>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">💨</div>
                    <div class="card-title">CO₂ Concentration</div>
                </div>
                <div class="metric-value" id="co2">--<span class="metric-unit">ppm</span></div>
                <div class="metric-label">Carbon Dioxide<span class="badge">MQ135 5V</span></div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">⚠️</div>
                    <div class="card-title">Gas Detection</div>
                </div>
                <div class="gas-alert alert-safe" id="gasAlert">
                    🛡️ Gas: <span id="gasStatus">SAFE</span> <span class="badge">MQ135</span>
                </div>
                <div class="gas-alert alert-safe" id="coAlert">
                    🔥 CO: <span id="coStatus">SAFE</span> <span class="badge">MQ9</span>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">📊</div>
                    <div class="card-title">System Information</div>
                </div>
                <div class="system-info">
                    <div><strong>📡 Network:</strong> <span id="wifiStatus">Connecting...</span></div>
                    <div><strong>🌐 IP Address:</strong> <span id="ipAddress">Loading...</span></div>
                    <div><strong>⏱️ Uptime:</strong> <span id="uptime">--</span></div>
                </div>
            </div>
        </div>
        
        <div class="last-update">
            <span class="pulse-dot"></span>
            Last updated: <span id="lastUpdate">--</span>
        </div>
    </div>

    <script>
        var updateProgressBar = function(elementId, value, max, colorClass) {
            var progressBar = document.getElementById(elementId);
            var percentage = Math.min((value / max) * 100, 100);
            progressBar.style.width = percentage + '%';
            progressBar.className = 'progress-fill ' + colorClass;
        };

        var updateData = function() {
            fetch('/api/data')
                .then(function(response) { return response.json(); })
                .then(function(data) {
                    document.getElementById('temperature').innerHTML = 
                        data.temperature.toFixed(1) + '<span class="metric-unit">°C</span>';
                    updateProgressBar('tempProgress', data.temperature, 50, 
                        data.temperature > 35 ? 'aqi-unhealthy' : 
                        data.temperature > 25 ? 'aqi-moderate' : 'aqi-good');
                    
                    document.getElementById('humidity').innerHTML = 
                        data.humidity.toFixed(0) + '<span class="metric-unit">%</span>';
                    updateProgressBar('humidityProgress', data.humidity, 100,
                        data.humidity > 80 ? 'aqi-unhealthy' :
                        data.humidity > 60 ? 'aqi-moderate' : 'aqi-good');
                    
                    document.getElementById('aqi').textContent = Math.round(data.aqi);
                    document.getElementById('aqiCategory').textContent = data.aqiCategory;
                    
                    var aqiProgress = Math.min((data.aqi / 200) * 100, 100);
                    var aqiProgressBar = document.getElementById('aqiProgress');
                    aqiProgressBar.style.width = aqiProgress + '%';
                    
                    var aqiColorClass = 'aqi-excellent';
                    if (data.aqi > 100) aqiColorClass = 'aqi-unhealthy';
                    else if (data.aqi > 50) aqiColorClass = 'aqi-moderate';
                    else if (data.aqi > 25) aqiColorClass = 'aqi-good';
                    aqiProgressBar.className = 'progress-fill ' + aqiColorClass;
                    
                    document.getElementById('co2').innerHTML = 
                        Math.round(data.co2) + '<span class="metric-unit">ppm</span>';
                    
                    var gasAlert = document.getElementById('gasAlert');
                    var gasStatus = document.getElementById('gasStatus');
                    if (data.gasDetected) {
                        gasAlert.className = 'gas-alert alert-danger';
                        gasStatus.innerHTML = '<strong>DETECTED!</strong>';
                    } else {
                        gasAlert.className = 'gas-alert alert-safe';
                        gasStatus.textContent = 'SAFE';
                    }
                    
                    var coAlert = document.getElementById('coAlert');
                    var coStatus = document.getElementById('coStatus');
                    if (data.coDetected) {
                        coAlert.className = 'gas-alert alert-danger';
                        coStatus.innerHTML = '<strong>DETECTED!</strong>';
                    } else {
                        coAlert.className = 'gas-alert alert-safe';
                        coStatus.textContent = 'SAFE';
                    }
                    
                    var statusIndicator = document.getElementById('statusIndicator');
                    var statusText = document.getElementById('statusText');
                    statusIndicator.className = 'status-indicator ';
                    
                    if (data.alertLevel === 0) {
                        statusIndicator.className += 'status-good';
                        statusText.innerHTML = '✅ System Healthy & Monitoring';
                    } else if (data.alertLevel === 1) {
                        statusIndicator.className += 'status-moderate';
                        statusText.innerHTML = '⚠️ Moderate Conditions Detected';
                    } else {
                        statusIndicator.className += 'status-dangerous';
                        statusText.innerHTML = '🚨 Alert! Environmental Warning';
                    }
                    
                    document.getElementById('wifiStatus').textContent = 
                        data.wifiConnected ? '🟢 Connected' : '🔴 Disconnected';
                    document.getElementById('ipAddress').textContent = data.ipAddress;
                    document.getElementById('uptime').textContent = data.uptime;
                    
                    var updateElement = document.getElementById('lastUpdate');
                    updateElement.style.opacity = '0.5';
                    setTimeout(function() {
                        updateElement.textContent = new Date().toLocaleTimeString();
                        updateElement.style.opacity = '1';
                    }, 200);
                })
                .catch(function(error) {
                    console.error('Error fetching data:', error);
                    document.getElementById('statusText').innerHTML = '❌ Connection Error - Retrying...';
                    var statusIndicator = document.getElementById('statusIndicator');
                    statusIndicator.className = 'status-indicator status-dangerous';
                });
        };
        
        updateData();
        setInterval(updateData, 2000);
    </script>
</body>
</html>
)rawliteral";

// Web Server Routes
void handleRoot() {
  server.send(200, "text/html", dashboard_html);
}

void handleAPI() {
  // Pre-calculate values to avoid multiple conversions
  int temp_int = (int)(temp_smoothed * 10);
  int humidity_int = (int)humidity_smoothed;
  int aqi_int = (int)calculated_aqi;
  int co2_int = (int)co2_ppm;
  int co_int = (int)(co_ppm * 10);
  
  unsigned long uptime = (millis() - startupTime) / 1000;
  String uptimeStr;
  if (uptime < 3600) {
    uptimeStr = String(uptime / 60) + "m " + String(uptime % 60) + "s";
  } else {
    uptimeStr = String(uptime / 3600) + "h " + String((uptime % 3600) / 60) + "m";
  }

  // Build JSON using proper string concatenation
  String json = "{";
  json += "\"temperature\":" + String(temp_int / 10.0, 1) + ",";
  json += "\"humidity\":" + String(humidity_int) + ",";
  json += "\"aqi\":" + String(aqi_int) + ",";
  json += "\"aqiCategory\":\"" + getAQICategory(calculated_aqi) + "\",";
  json += "\"co2\":" + String(co2_int) + ",";
  json += "\"co\":" + String(co_int / 10.0, 1) + ",";
  json += "\"mq135_analog\":" + String(mq135_analog) + ",";
  
  // Fix for boolean values
  json += "\"gasDetected\":";
  json += (mq135_digital == LOW) ? "true" : "false";
  json += ",";
  
  json += "\"coDetected\":";
  json += (mq9_digital == LOW) ? "true" : "false";
  json += ",";
  
  json += "\"alertLevel\":" + String(currentAlert) + ",";
  
  json += "\"wifiConnected\":";
  json += (WiFi.status() == WL_CONNECTED) ? "true" : "false";
  json += ",";
  
  json += "\"ipAddress\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"uptime\":\"" + uptimeStr + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/api/data", handleAPI);
  server.begin();
  Serial.println("Web server started!");
  Serial.print("Dashboard URL: http://");
  Serial.println(WiFi.localIP());
}

// ========================================
// MAIN PROGRAM FUNCTIONS
// ========================================

// System Initialization
void setup() {
  Serial.begin(115200);
  delay(100);
  
  startupTime = millis();
  
  // Initialize display
  if (!display.begin(0x3C)) {
    Serial.println("OLED initialization failed!");
    while (1);
  }
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  
  // Initialize pins
  pinMode(MQ135_DO, INPUT_PULLUP);
  pinMode(MQ9_DO, INPUT_PULLUP);
  pinMode(TRAFFIC_LIGHT_GREEN, OUTPUT);
  pinMode(TRAFFIC_LIGHT_YELLOW, OUTPUT);
  pinMode(TRAFFIC_LIGHT_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Startup animation sequence
  for (int i = 0; i <= 40; i += 5) {
    progressBar = i;
    loadingText = "Initializing";
    updateStartupDisplay();
    delay(200);
  }
  
  // Initialize DHT
  loadingText = "DHT22 Sensor";
  updateStartupDisplay();
  dht.begin();
  delay(500);
  progressBar = 50;
  updateStartupDisplay();
  
  // Connect WiFi
  loadingText = "WiFi Connect";
  updateStartupDisplay();
  connectWiFi();
  
  // Setup Web Server
  if (WiFi.status() == WL_CONNECTED) {
    progressBar = 85;
    loadingText = "Web Server";
    updateStartupDisplay();
    setupWebServer();
  }
  
  // Final setup
  progressBar = 90;
  loadingText = "Finalizing";
  updateStartupDisplay();
  delay(500);
  
  progressBar = 100;
  loadingText = "Ready";
  updateStartupDisplay();
  delay(1000);
  
  // Play startup completion tone
  playStartupTone();
  
  setTrafficLight(GOOD);
  systemReady = true;
  currentMode = MODE_TEMP_HUMIDITY;
  lastModeSwitch = millis();
  
  Serial.println("EnviroGuardian v1.0 Ready!");
  Serial.println("Author: Ibrahim Hasan");
  Serial.println("MQ135: Full analog + digital readings (5V operation)");
  Serial.println("MQ9: Gas detection readings");
  Serial.println("Traffic Light: D0=Green, D8=Yellow, D3=Red");
  Serial.println("Audio: Dual active buzzers for enhanced alerts");
  Serial.println("===========================================");
  Serial.println("📊 SENSOR CALIBRATION INFO:");
  Serial.println("• MQ135 readings should be 2-15 for clean air (5V)");
  Serial.println("• If CO2 stays at 400ppm, try breathing near sensor");
  Serial.println("• First 2-3 minutes may show unstable readings");
  Serial.printf("• Web Dashboard: http://%s\n", WiFi.localIP().toString().c_str());
  Serial.println("===========================================");
}

// Main Program Loop
void loop() {
  if (!systemReady) return;
  
  // Read sensors
  readSensors();
  
  // Handle alerts
  handleAlerts();
  
  // Auto-cycle display modes every 10 seconds
  if (millis() - lastModeSwitch > 10000) {
    switch(currentMode) {
      case MODE_TEMP_HUMIDITY:
        currentMode = MODE_AIR_QUALITY;
        break;
      case MODE_AIR_QUALITY:
        currentMode = MODE_SYSTEM_INFO;
        break;
      case MODE_SYSTEM_INFO:
        currentMode = MODE_TEMP_HUMIDITY;
        break;
      default:
        currentMode = MODE_TEMP_HUMIDITY;
        break;
    }
    lastModeSwitch = millis();
  }
  
  // Update display based on current mode
  switch(currentMode) {
    case MODE_TEMP_HUMIDITY:
      displayTempHumidity();
      break;
    case MODE_AIR_QUALITY:
      displayAirQuality();
      break;
    case MODE_SYSTEM_INFO:
      displaySystemInfo();
      break;
  }
  
  // Check WiFi connection only if actually disconnected
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 30000) { // Try every 30 seconds
      Serial.println("WiFi disconnected, attempting reconnection...");
      connectWiFi();
      lastReconnectAttempt = millis();
    }
  }
  
  // Handle web server requests
  server.handleClient();
  
  // Print to serial with calibrated values
  Serial.printf("Mode: %d | Temp: %.1f°C | Humidity: %.1f%% | AQI: %.0f | CO2: %.0f ppm | CO: %.1f ppm | MQ135: %d | MQ9: %d | Alert: %d\n", 
                currentMode, temp_smoothed, humidity_smoothed, calculated_aqi, co2_ppm, co_ppm, mq135_analog, mq9_digital == LOW ? 1 : 0, currentAlert);

  delay(500); // Faster updates for smoother animations
}
