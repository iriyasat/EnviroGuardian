/* 
  EnviroGuardian - Air Quality Monitoring System
  Connected to Arduino IoT Cloud
  https://create.arduino.cc/cloud/things/0a89a51a-0a04-45d0-a16e-b2d5174277d0 
  
  Arduino IoT Cloud Variables:
  String airQualityStatus;
  String coStatus;
  String currentDate;
  String currentTime;
  String dayOfWeek;
  String humidStatus;
  String ipAddress;
  String systemUptime;
  String tempStatus;
  String wifiStatus;
  float humidity;
  float temperature;
  int airQuality;
  bool coDetected;
*/

//==================== LIBRARIES ====================
#include "thingProperties.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <DHT.h>
#include <time.h>

//==================== CONSTANTS & PINS ====================
// Project name
#define PROJECT_NAME "EnviroGuardian"

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1   // Reset pin # (or -1 if sharing Arduino reset pin)

// Alert system pins
#define GREEN_LED_PIN 14   // D5 on NodeMCU
#define YELLOW_LED_PIN 12  // D6 on NodeMCU
#define RED_LED_PIN 13     // D7 on NodeMCU
#define BUZZER_PIN 15      // D8 on NodeMCU

// DHT22 sensor
#define DHTPIN 0           // D3 on NodeMCU
#define DHTTYPE DHT22      // DHT 22 (AM2302)

// MQ Gas sensors
#define MQ135_ANALOG_PIN A0     // A0 on NodeMCU
#define MQ135_DIGITAL_PIN 2     // D4 on NodeMCU
#define MQ9_DIGITAL_PIN 16      // D0 on NodeMCU

// Time settings for NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 21600;      // Bangladesh is UTC+6 (6*3600)
const int daylightOffset_sec = 0;      // No DST in Bangladesh

//==================== OBJECTS ====================
// Initialize display (using I2C)
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

//==================== GLOBAL VARIABLES ====================
// Display control
unsigned long previousDisplayMillis = 0;
const long displayInterval = 5000;     // 5 seconds per screen
int currentDisplay = 0;                // Current display mode

// Sensor reading timing
unsigned long previousSensorMillis = 0;
const long sensorInterval = 2000;      // Read sensors every 2 seconds

// Buzzer control
bool buzzerActive = false;
unsigned long buzzerStartTime = 0;
unsigned long buzzerDuration = 0;
int buzzerPattern = 0;

// MQ Sensors Calibration
float mq135_r0 = 76.63;                // Default R0 value for clean air
bool sensorCalibrated = false;
unsigned long startupTime = 0;

// System start time (for uptime calculation)
unsigned long systemStartTime = 0;

//==================== SETUP FUNCTION ====================
void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  
  // This delay gives the chance to wait for a Serial Monitor
  delay(1500); 
  
  // Initialize hardware pins
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MQ135_DIGITAL_PIN, INPUT);
  pinMode(MQ9_DIGITAL_PIN, INPUT);
  
  // Turn off all LEDs and buzzer
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize the OLED display
  display.begin(0x3C, true);
  display.clearDisplay();
  
  // Initialize DHT sensor
  dht.begin();
  
  // Record system start time
  systemStartTime = millis();
  
  // Initialize IoT Cloud Properties
  initProperties();
  
  // Run startup animation
  startupAnimation();
  
  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  // Set debug message level
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  
  // Show connection screen
  connectingAnimation();
  
  // Configure time from NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Update system status
  updateSystemInfo();
  
  // Success indication - all LEDs blink together
  for (int i = 0; i < 3; i++) {
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(80);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(120);
  }
  
  // Calibrate sensors if needed
  startupTime = millis();
  if (!sensorCalibrated) {
    calibrateMQ135();
  }
  
  // Set initial display mode switch time
  previousDisplayMillis = millis();
}

//==================== MAIN LOOP ====================
void loop() {
  // Handle Arduino IoT Cloud updates
  ArduinoCloud.update();
  
  // Get current time
  unsigned long currentMillis = millis();
  
  // Handle buzzer patterns
  buzzerHandler(currentMillis);
  
  // Read sensors at regular intervals
  if (currentMillis - previousSensorMillis >= sensorInterval) {
    previousSensorMillis = currentMillis;
    
    // Read sensors and update readings
    readSensors();
    
    // Update IoT Cloud variables
    updateCloudVariables();
    
    // Update system information
    updateSystemInfo();
  }
  
  // Update display at regular intervals
  if (currentMillis - previousDisplayMillis >= displayInterval) {
    previousDisplayMillis = currentMillis;
    
    // Cycle through displays
    currentDisplay = (currentDisplay + 1) % 4;
  }
  
  // Display current screen
  switch (currentDisplay) {
    case 0:
      displayClockScreen();
      break;
    case 1:
      displayTempHumidScreen();
      break;
    case 2:
      displayAirQualityScreen();
      break;
    case 3:
      displaySystemInfoScreen();
      break;
  }
  
  // Short delay to prevent display flicker
  delay(50);
}

//==================== SENSOR FUNCTIONS ====================
// Read all sensors and update readings
void readSensors() {
  // Read DHT22 sensor
  float newTemp = dht.readTemperature();
  float newHumid = dht.readHumidity();
  
  if (!isnan(newTemp) && !isnan(newHumid)) {
    temperature = newTemp;
    humidity = newHumid;
    
    // Update status strings
    if (temperature > 35) {
      tempStatus = "HAZARDOUS";
    } else if (temperature > 30 || temperature < 15) {
      tempStatus = "WARNING";
    } else {
      tempStatus = "NORMAL";
    }
    
    if (humidity > 70) {
      humidStatus = "TOO WET";
    } else if (humidity < 30) {
      humidStatus = "TOO DRY";
    } else {
      humidStatus = "NORMAL";
    }
  }
  
  // Read MQ135 sensor (Air Quality)
  airQuality = readMQ135();
  
  // Update air quality status
  if (airQuality < 700) {
    airQualityStatus = "GOOD";
  } else if (airQuality < 1000) {
    airQualityStatus = "FAIR";
  } else if (airQuality < 1500) {
    airQualityStatus = "POOR";
  } else {
    airQualityStatus = "DANGER";
  }
  
  // Read MQ9 sensor (CO detection)
  coDetected = !digitalRead(MQ9_DIGITAL_PIN);
  coStatus = coDetected ? "ALERT!" : "Normal";
  
  // Update alert system
  updateAlertSystem();
}

// Calculate MQ135 readings in ppm
int readMQ135() {
  int adcValue = analogRead(MQ135_ANALOG_PIN);
  float voltage = adcValue * (3.3 / 1023.0);
  
  // Calculate resistance ratio (using calibrated R0)
  float rs = ((3.3 * 10.0) / voltage) - 10.0; // Assuming 10K load resistor
  float ratio = rs / mq135_r0;
  
  // Convert to ppm (approximate - requires specific calibration for accuracy)
  float ppm = 116.6020682 * pow(ratio, -2.769034857);
  
  return ppm;
}

// Calibrate MQ135 sensor
void calibrateMQ135() {
  float rs_avg = 0.0;
  
  // Take multiple readings for accuracy
  for (int i = 0; i < 10; i++) {
    int adcValue = analogRead(MQ135_ANALOG_PIN);
    float voltage = adcValue * (3.3 / 1023.0);
    float rs = ((3.3 * 10.0) / voltage) - 10.0; // Assuming 10K load resistor
    rs_avg += rs;
    delay(100);
  }
  rs_avg /= 10.0;
  
  // In clean air, Rs/R0 = 3.6 for MQ135
  mq135_r0 = rs_avg / 3.6;
  
  Serial.print("MQ135 R0 calibrated to: ");
  Serial.println(mq135_r0);
  
  sensorCalibrated = true;
}

//==================== ALERT SYSTEM FUNCTIONS ====================
// Update the alert system based on sensor readings
void updateAlertSystem() {
  // Turn off all LEDs first
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  
  // Handle critical alerts first (CO detection)
  if (coDetected) {
    // Carbon monoxide detected - highest priority!
    digitalWrite(RED_LED_PIN, HIGH);
    activateBuzzer(3);  // Critical alert
    return;
  }
  
  // Air quality alerts
  if (airQuality > 1500) {
    // Dangerous air quality - red alert
    digitalWrite(RED_LED_PIN, HIGH);
    activateBuzzer(2);  // Warning pattern
    return;
  } else if (airQuality > 1000) {
    // Poor air quality - yellow alert
    digitalWrite(YELLOW_LED_PIN, HIGH);
    activateBuzzer(1);  // Attention pattern
    return;
  }
  
  // Temperature-based alerts
  if (temperature > 35) {
    // Extreme heat - red alert
    digitalWrite(RED_LED_PIN, HIGH);
    activateBuzzer(2);  // Warning pattern
  } else if (temperature > 30 || temperature < 15) {
    // Too hot/cold - yellow alert
    digitalWrite(YELLOW_LED_PIN, HIGH);
  } else if (humidity > 70 || humidity < 30) {
    // Humidity concerns - yellow alert
    digitalWrite(YELLOW_LED_PIN, HIGH);
  } else {
    // Everything is good - green LED
    digitalWrite(GREEN_LED_PIN, HIGH);
  }
}

// Activate buzzer with different patterns based on alert level
void activateBuzzer(int alertLevel) {
  // Already active? Don't interrupt current alert
  if (buzzerActive) return;
  
  buzzerActive = true;
  buzzerStartTime = millis();
  
  switch (alertLevel) {
    case 1: // Attention - single beep
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerDuration = 200;
      buzzerPattern = 0;
      break;
      
    case 2: // Warning - double beep pattern
      buzzerPattern = 1;  // Set pattern for the buzzerHandler
      buzzerDuration = 1000;
      break;
      
    case 3: // Critical - rapid beep pattern
      buzzerPattern = 2;  // Set pattern for the buzzerHandler
      buzzerDuration = 2000;
      break;
      
    default:
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerDuration = 100;
      buzzerPattern = 0;
      break;
  }
}

// Handle buzzer patterns over time
void buzzerHandler(unsigned long currentMillis) {
  // Not active, nothing to do
  if (!buzzerActive) return;
  
  // Pattern handling
  if (buzzerPattern == 1) {  // Double beep pattern
    long patternTime = (currentMillis - buzzerStartTime) % 500;
    if (patternTime < 100) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else if (patternTime < 200) {
      digitalWrite(BUZZER_PIN, LOW);
    } else if (patternTime < 300) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
  else if (buzzerPattern == 2) {  // Rapid beep pattern
    long patternTime = (currentMillis - buzzerStartTime) % 200;
    if (patternTime < 100) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
  
  // Check if duration passed
  if (currentMillis - buzzerStartTime >= buzzerDuration) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
    buzzerPattern = 0;
  }
}

// Simple beep function
void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

// Beep pattern function
void beepPattern(int beeps, int onDuration, int offDuration) {
  for (int i = 0; i < beeps; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(onDuration);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < beeps - 1) { // No delay after the last beep
      delay(offDuration);
    }
  }
}

//==================== DISPLAY FUNCTIONS ====================
// Display Mode 1: Clock & Date
void displayClockScreen() {
  struct tm timeinfo;
  bool timeAvailable = getLocalTime(&timeinfo);
  
  display.clearDisplay();
  
  // Draw fancy border
  display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 3, SH110X_WHITE);
  
  if (!timeAvailable) {
    display.setTextSize(1);
    display.setCursor(15, 25);
    display.println("Time Unavailable");
    display.setCursor(15, 40);
    display.println("Check WiFi connection");
  } else {
    // Format and update date strings for IoT Cloud
    char dateBuffer[16];
    strftime(dateBuffer, sizeof(dateBuffer), "%b %d, %Y", &timeinfo);
    currentDate = String(dateBuffer);
    
    char timeBuffer[10];
    strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeinfo);
    currentTime = String(timeBuffer);
    
    char dowBuffer[10];
    strftime(dowBuffer, sizeof(dowBuffer), "%A", &timeinfo);
    dayOfWeek = String(dowBuffer);
    
    // Display date at top
    display.setTextSize(1);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(dateBuffer, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 5);
    display.println(dateBuffer);
    
    // Display time (large)
    display.setTextSize(2);
    display.getTextBounds(timeBuffer, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 25);
    display.println(timeBuffer);
    
    // Display day of week
    display.setTextSize(1);
    display.getTextBounds(dowBuffer, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 50);
    display.println(dowBuffer);
  }
  
  // Status indicators
  drawStatusIcons();
  display.display();
}

// Display Mode 2: Temperature & Humidity
void displayTempHumidScreen() {
  display.clearDisplay();
  
  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Temperature & Humidity");
  display.drawLine(0, 10, SCREEN_WIDTH-1, 10, SH110X_WHITE);
  
  // Temperature section
  display.setTextSize(2);
  display.setCursor(10, 16);
  display.print(temperature, 1);
  display.print((char)247); // Degree symbol
  display.print("C");
  
  // Temperature status
  display.setTextSize(1);
  
  // Status box
  int statusWidth = tempStatus.length() * 6;
  if (tempStatus == "NORMAL") {
    drawStatusBox(SCREEN_WIDTH-statusWidth-4, 18, statusWidth+4, 11, tempStatus, false);
  } else if (tempStatus == "WARNING") {
    drawStatusBox(SCREEN_WIDTH-statusWidth-4, 18, statusWidth+4, 11, tempStatus, true);
  } else {
    drawStatusBox(SCREEN_WIDTH-statusWidth-4, 18, statusWidth+4, 11, tempStatus, true);
  }
  
  // Separator
  display.drawLine(0, 36, SCREEN_WIDTH-1, 36, SH110X_WHITE);
  
  // Humidity section
  display.setTextSize(2);
  display.setCursor(10, 42);
  display.print(humidity, 1);
  display.print("%");
  
  // Humidity status
  display.setTextSize(1);
  
  // Status box for humidity
  statusWidth = humidStatus.length() * 6;
  if (humidStatus == "NORMAL") {
    drawStatusBox(SCREEN_WIDTH-statusWidth-4, 44, statusWidth+4, 11, humidStatus, false);
  } else {
    drawStatusBox(SCREEN_WIDTH-statusWidth-4, 44, statusWidth+4, 11, humidStatus, true);
  }
  
  // Status indicators
  drawStatusIcons();
  display.display();
}

// Display Mode 3: Air Quality Data
void displayAirQualityScreen() {
  display.clearDisplay();
  
  // Header
  display.setTextSize(1);
  display.setCursor(10, 0);
  display.println("Air Quality Monitor");
  display.drawLine(0, 10, SCREEN_WIDTH-1, 10, SH110X_WHITE);
  
  // MQ135 section
  display.setCursor(5, 14);
  display.println("Air Quality (ppm):");
  
  // Value
  display.setTextSize(2);
  display.setCursor(15, 24);
  display.print(airQuality);
  
  // Status
  display.setTextSize(1);
  
  // Status box
  int statusWidth = airQualityStatus.length() * 6;
  if (airQualityStatus == "GOOD") {
    drawStatusBox(SCREEN_WIDTH-statusWidth-4, 26, statusWidth+4, 11, airQualityStatus, false);
  } else if (airQualityStatus == "FAIR") {
    drawStatusBox(SCREEN_WIDTH-statusWidth-4, 26, statusWidth+4, 11, airQualityStatus, false);
  } else {
    drawStatusBox(SCREEN_WIDTH-statusWidth-4, 26, statusWidth+4, 11, airQualityStatus, true);
  }
  
  // Separator
  display.drawLine(0, 38, SCREEN_WIDTH-1, 38, SH110X_WHITE);
  
  // CO Detector section
  display.setCursor(5, 42);
  display.println("Carbon Monoxide:");
  
  // Status
  display.setTextSize(2);
  if (coDetected) {
    display.setCursor(15, 52);
    display.println("ALERT!");
    
    // Warning box
    drawStatusBox(SCREEN_WIDTH-60, 52, 56, 16, "DANGER", true);
  } else {
    display.setCursor(15, 52);
    display.println("Normal");
  }
  
  // Status indicators
  drawStatusIcons();
  display.display();
}

// Display Mode 4: System Information
void displaySystemInfoScreen() {
  display.clearDisplay();
  
  // Header
  display.setTextSize(1);
  display.setCursor(10, 0);
  display.println("System Information");
  display.drawLine(0, 10, SCREEN_WIDTH-1, 10, SH110X_WHITE);
  
  // WiFi Status
  display.setCursor(5, 14);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi: ");
    
    // Limit SSID length to fit the screen
    String displaySsid = WiFi.SSID();
    if (displaySsid.length() > 10) {
      displaySsid = displaySsid.substring(0, 8) + "..";
    }
    display.println(displaySsid);
    
    // IP Address
    display.setCursor(5, 24);
    display.print("IP: ");
    display.println(WiFi.localIP().toString());
  } else {
    display.println("WiFi: Disconnected");
  }
  
  // IoT Cloud Status
  display.setCursor(5, 34);
  display.print("IoT: ");
  display.println(ArduinoCloud.connected() ? "Connected" : "Disconnected");
  
  // System uptime
  display.setCursor(5, 44);
  display.print("Uptime: ");
  display.println(systemUptime);
  
  // Sensors status
  display.setCursor(5, 54);
  display.print("Sensors: ");
  display.println(sensorCalibrated ? "Calibrated" : "Warming up");
  
  // Status indicators
  drawStatusIcons();
  display.display();
}

// Draw status indicator icons in top-right corner
void drawStatusIcons() {
  // WiFi status
  if (WiFi.status() == WL_CONNECTED) {
    // Draw WiFi icon
    for (int i = 0; i < 3; i++) {
      display.drawCircle(120, 4, i+2, SH110X_WHITE);
    }
  }
  
  // Alert status
  if (coDetected) {
    // CO Alert - critical!
    display.fillCircle(4, 4, 3, SH110X_WHITE);
    display.setTextColor(SH110X_BLACK);
    display.setCursor(2, 2);
    display.print("!");
    display.setTextColor(SH110X_WHITE);
  } else if (airQuality > 1500 || temperature > 35) {
    // Danger condition
    display.fillCircle(4, 4, 3, SH110X_WHITE);
  } else if (airQuality > 1000 || temperature > 30 || temperature < 15 ||
             humidity > 70 || humidity < 30) {
    // Warning condition
    display.drawCircle(4, 4, 3, SH110X_WHITE);
  }
}

// Draw a status box with text
void drawStatusBox(int x, int y, int w, int h, String text, bool warning) {
  if (warning) {
    display.fillRect(x, y, w, h, SH110X_WHITE);
    display.setTextColor(SH110X_BLACK);
    display.setCursor(x + 2, y + 2);
    display.print(text);
    display.setTextColor(SH110X_WHITE);
  } else {
    display.drawRect(x, y, w, h, SH110X_WHITE);
    display.setCursor(x + 2, y + 2);
    display.print(text);
  }
}

//==================== ANIMATION FUNCTIONS ====================
// Beautiful startup animation
void startupAnimation() {
  // Clear display
  display.clearDisplay();
  
  // Part 1: Logo reveal with concentric rectangles
  for (int i = 0; i < SCREEN_HEIGHT/2; i+=2) {
    display.clearDisplay();
    display.drawRect(SCREEN_WIDTH/2-i, SCREEN_HEIGHT/2-i, i*2, i*2, SH110X_WHITE);
    display.display();
    
    // Synchronized beep
    if (i % 8 == 0) {
      beep(10 + i);
    }
    delay(20);
  }
  
  // Part 2: Logo flash
  for (int i = 0; i < 3; i++) {
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_WHITE);
    display.display();
    beep(30);
    delay(50);
    
    display.clearDisplay();
    display.display();
    delay(50);
  }
  
  // Part 3: Display logo with animation
  display.clearDisplay();
  
  // Draw logo text letter by letter
  const char* line1 = "Enviro";
  const char* line2 = "Guardian";
  
  // First line animation
  display.setTextSize(2);
  for (int i = 0; i < strlen(line1); i++) {
    char tempStr[10] = {0};
    strncpy(tempStr, line1, i+1);
    
    display.clearDisplay();
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(tempStr, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 15);
    display.println(tempStr);
    display.display();
    
    beep(5);
    delay(70);
  }
  
  // Second line animation
  for (int i = 0; i < strlen(line2); i++) {
    char tempStr[15] = {0};
    strncpy(tempStr, line2, i+1);
    
    display.clearDisplay();
    int16_t x1, y1;
    uint16_t w, h;
    
    // First line
    display.setTextSize(2);
    display.getTextBounds(line1, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 15);
    display.println(line1);
    
    // Second line animation
    display.getTextBounds(tempStr, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 35);
    display.println(tempStr);
    display.display();
    
    beep(10);
    delay(70);
  }
  
  // Add subtitle with fade-in effect
  const char* subtitle = "Air Quality Monitor";
  display.drawLine(5, 55, 123, 55, SH110X_WHITE);
  display.display();
  beep(20);
  delay(300);
  
  display.setTextSize(1);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(subtitle, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 58);
  display.println(subtitle);
  display.display();
  
  // Final startup tone
  beepPattern(3, 50, 50);
  delay(100);
  beep(200);
  
  delay(1000);
}

// Connecting to WiFi animation
void connectingAnimation() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(PROJECT_NAME);
  display.drawLine(0, 9, SCREEN_WIDTH-1, 9, SH110X_WHITE);
  
  // Connection animation
  int dotCount = 0;
  digitalWrite(YELLOW_LED_PIN, HIGH); // Yellow means connecting
  
  // Draw progress bar border
  display.drawRect(0, 25, SCREEN_WIDTH, 10, SH110X_WHITE);
  display.display();
  
  int animPosition = 0;
  
  // Show animation for a few seconds
  unsigned long startTime = millis();
  while (millis() - startTime < 5000 && WiFi.status() != WL_CONNECTED) {
    // Update progress bar with a moving segment
    display.fillRect(2, 27, SCREEN_WIDTH-4, 6, SH110X_BLACK);
    animPosition = (animPosition + 5) % (SCREEN_WIDTH - 20);
    display.fillRect(2 + animPosition, 27, 15, 6, SH110X_WHITE);
    
    // Connection status with dots
    display.fillRect(0, 38, SCREEN_WIDTH, 18, SH110X_BLACK);
    display.setCursor(0, 40);
    display.print("Connecting to IoT Cloud");
    for (int i = 0; i < dotCount; i++) {
      display.print(".");
    }
    dotCount = (dotCount + 1) % 4;
    
    display.display();
    
    // Make connecting sounds
    if (dotCount == 0) {
      beep(5);
    }
    
    delay(150);
  }
  
  // Show connection status
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(PROJECT_NAME);
  display.drawLine(0, 9, SCREEN_WIDTH-1, 9, SH110X_WHITE);
  
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
    
    display.setCursor(5, 20);
    display.println("IoT Cloud Connected!");
    display.setCursor(5, 35);
    display.print("IP: ");
    display.println(WiFi.localIP().toString());
    
    // Connected sound
    beepPattern(3, 50, 50);
  } else {
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
    
    display.setCursor(5, 20);
    display.println("Connection Failed");
    display.setCursor(5, 35);
    display.println("Continue in offline mode");
    
    // Error tone
    beepPattern(2, 200, 100);
  }
  
  display.display();
  delay(2000);
  
  // Turn off LEDs
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
}

//==================== IOT CLOUD FUNCTIONS ====================
// Update IoT Cloud variables
void updateCloudVariables() {
  // Time and date variables are updated in displayClockScreen()
  
  // Update system info
  updateSystemInfo();
}

// Update system information
void updateSystemInfo() {
  // Update WiFi status
  if (WiFi.status() == WL_CONNECTED) {
    wifiStatus = "Connected";
    ipAddress = WiFi.localIP().toString();
  } else {
    wifiStatus = "Disconnected";
    ipAddress = "0.0.0.0";
  }
  
  // Update system uptime
  unsigned long uptimeSeconds = millis() / 1000;
  int days = uptimeSeconds / 86400;
  uptimeSeconds %= 86400;
  int hours = uptimeSeconds / 3600;
  uptimeSeconds %= 3600;
  int minutes = uptimeSeconds / 60;
  int seconds = uptimeSeconds % 60;
  
  char uptimeStr[20];
  if (days > 0) {
    sprintf(uptimeStr, "%dd %02d:%02d:%02d", days, hours, minutes, seconds);
  } else {
    sprintf(uptimeStr, "%02d:%02d:%02d", hours, minutes, seconds);
  }
  systemUptime = String(uptimeStr);
}
