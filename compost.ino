// --- COMPOSTING IOT MONITOR - ESP8266 SKETCH (UBIDOTS INTEGRATION) ---
// This sketch reads temperature (DS18B20) and moisture (Analog Capacitive Sensor)
// and sends the data to Ubidots over Wi-Fi, which has a dedicated mobile app.

// 1. INCLUDE REQUIRED LIBRARIES
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
// ---------------------------------------------------------------------------------
// CRITICAL FIX FOR LIBRARY ERROR: 
// Reverting to the standard include name with angle brackets to resolve the library path issue.
#include <Ubidots.h> 
// ---------------------------------------------------------------------------------

// 2. WIFI & UBIDOTS CONFIGURATION (UPDATE THESE)
const char* WIFI_SSID = "YOUR_WIFI_SSID";          // Your WiFi Network Name
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";  // Your WiFi Password

// Ubidots Account Configuration
// Go to https://ubidots.com/ and get your account token from your profile page.
const char* UBIDOTS_TOKEN = "YOUR_UBIDOTS_ACCOUNT_TOKEN"; 
const char* DEVICE_LABEL = "compost-monitor"; // A unique label for this specific device
const char* TEMPERATURE_LABEL = "temperature"; // The variable name for temperature
const char* MOISTURE_LABEL = "moisture";     // The variable name for moisture

// Delay between sending updates (in milliseconds). 
const long readingInterval = 20000; // 20 seconds (20000 ms)
long lastReadingTime = 0;

// 3. SENSOR PIN DEFINITIONS & ALERT CONFIG
// DS18B20 Temperature Sensor (Data pin connected to D2 on ESP8266)
// NOTE: On NodeMCU boards, D2 is GPIO 4
#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Capacitive Moisture Sensor (Analog pin A0 on ESP8266)
#define MOISTURE_PIN A0

// Built-in LED for visual alerts (usually GPIO 2 / D4 on NodeMCU)
#define ALERT_LED_PIN 2

// 4. OPTIMAL COMPOSTING THRESHOLDS
// Composting is active in the THERMOPHILIC range (55C - 65C)
const float MIN_TEMP_C = 50.0; // Alert if temperature drops below this (needs turning)
const float MAX_MOISTURE_PCT = 65; // Alert if moisture is above this (too wet, anaerobic risk)
const float MIN_MOISTURE_PCT = 40; // Alert if moisture is below this (too dry)

// Calibration values for the moisture sensor
const int AIR_VALUE = 850;   // Sensor value when completely dry (in air)
const int WATER_VALUE = 300; // Sensor value when completely wet (in water)

// Initialize Ubidots Client
// NOTE: The Ubidots class is now defined in Ubidots.h
Ubidots ubidots(UBIDOTS_TOKEN);

// --- SETUP FUNCTION ---
void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Compost IoT Monitor Initializing (Ubidots) ---");

  // Setup the onboard LED pin as an output
  pinMode(ALERT_LED_PIN, OUTPUT);
  digitalWrite(ALERT_LED_PIN, HIGH); // Turn off LED (HIGH is usually OFF for onboard ESP LEDs)

  // Initialize temperature sensor
  sensors.begin();
  Serial.print("DS18B20 Sensor Count: ");
  Serial.println(sensors.getDeviceCount());
  
  // Set up Wi-Fi using the Ubidots library function
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  Serial.print("Ubidots client initialized. IP Address: ");
  Serial.println(WiFi.localIP());
}

// --- UTILITY FUNCTION ---
// Converts Celsius to Fahrenheit
float toFahrenheit(float celsius) {
  return celsius * 9.0 / 5.0 + 32.0;
}

// Function to flash the LED to indicate an alert condition
void triggerAlert() {
  // Flash rapidly for 1 second
  for (int i = 0; i < 10; i++) {
    digitalWrite(ALERT_LED_PIN, LOW); // LED ON
    delay(50);
    digitalWrite(ALERT_LED_PIN, HIGH); // LED OFF
    delay(50);
  }
}

// --- READING FUNCTIONS ---

// Function to read temperature from DS18B20
float getTemperature() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensors.getTempCByIndex(0); // Get the temperature in Celsius of the first sensor
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: DS18B20 disconnected or error reading!");
    return -99.0; // Error value
  }
  return tempC;
}

// Function to read and map moisture percentage
int getMoisturePercentage() {
  int rawValue = analogRead(MOISTURE_PIN);
  
  // Map the raw analog value to a percentage (0-100)
  int moisturePercent = map(rawValue, AIR_VALUE, WATER_VALUE, 0, 100);
  
  // Constrain the value between 0 and 100
  moisturePercent = constrain(moisturePercent, 0, 100); 
  
  Serial.print("Raw Moisture: ");
  Serial.print(rawValue);
  Serial.print(", Moisture %: ");
  Serial.println(moisturePercent);
  
  return moisturePercent;
}


// --- UBIDOTS DATA TRANSMISSION ---
void sendDataToUbidots(float tempC, int moisturePercent) {
  // Check if connected and try to reconnect if not
  if (!ubidots.isConnected()) {
    Serial.println("Ubidots client not connected. Attempting reconnection...");
    ubidots.reconnect();
  }

  if (ubidots.isConnected()) {
    // Add the values to the payload (variable label, value)
    ubidots.add(TEMPERATURE_LABEL, tempC);
    ubidots.add(MOISTURE_LABEL, moisturePercent);
    // Optionally add Fahrenheit value too
    ubidots.add("temp_f", toFahrenheit(tempC)); 

    Serial.print("Sending data to Ubidots...");
    
    // Send the data using the device label
    if (ubidots.sendAll(DEVICE_LABEL)) {
      Serial.println(" Data sent successfully.");
    } else {
      Serial.println(" ERROR: Failed to send data to Ubidots.");
    }
  } else {
    Serial.println("Failed to connect to Ubidots after retry.");
  }
}

// --- MAIN LOOP ---
void loop() {
  // Check if it's time to take a new reading and send data
  if (millis() - lastReadingTime >= readingInterval) {
    
    // 1. READ SENSOR DATA
    float tempC = getTemperature();
    int moisturePercent = getMoisturePercentage();
    
    // 2. PRINT TO SERIAL
    Serial.println("--- Reading ---");
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" C");
    Serial.print("Moisture: ");
    Serial.print(moisturePercent);
    Serial.println(" %");

    // 3. CHECK FOR CRITICAL CONDITIONS AND ALERT
    if (tempC != -99.0 && (tempC < MIN_TEMP_C || moisturePercent < MIN_MOISTURE_PCT || moisturePercent > MAX_MOISTURE_PCT)) {
      Serial.println("!!! CRITICAL ALERT: Compost outside optimal range. Flashing LED. !!!");
      triggerAlert(); // Flash the built-in LED
    } else if (tempC != -99.0) {
      Serial.println("Status: Optimal range. Turning off LED.");
      digitalWrite(ALERT_LED_PIN, HIGH); // Ensure LED is off
    }
    
    // 4. SEND DATA
    if (tempC != -99.0) {
      sendDataToUbidots(tempC, moisturePercent);
    }

    lastReadingTime = millis();
  }
  
  // Short delay to avoid flooding the loop unnecessarily
  delay(100);
}
