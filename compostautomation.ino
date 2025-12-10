#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "DHT.h"

#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define MOISTURE_PIN A0
#define STIR_SERVO_PIN 9
#define VALVE_SERVO_PIN 10

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo stirServo;
Servo valveServo;

// Adjust after calibration
int dryValue = 1000;
int wetValue = 300;

// Thresholds & Timers
int moistureThreshold = 35; // if below, stir & open valve
unsigned long lastStirTime = 0;
unsigned long stirInterval = 6UL * 60UL * 60UL * 1000UL; // every 6 hours
bool stirredRecently = false;

void setup() {
  Serial.begin(9600);
  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Compost Monitor");
  delay(2000);
  lcd.clear();

  stirServo.attach(STIR_SERVO_PIN);
  valveServo.attach(VALVE_SERVO_PIN);

  stirServo.write(0);
  valveServo.write(0); // valve closed initially
}

void stirCompost() {
  Serial.println(">> Stirring compost...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stirring compost");

  for (int i = 0; i < 3; i++) {
    stirServo.write(0);
    delay(500);
    stirServo.write(90);
    delay(500);
    stirServo.write(180);
    delay(500);
  }

  stirServo.write(0);
  lcd.setCursor(0, 1);
  lcd.print("Done!");
  delay(1000);
  lcd.clear();
}

void openValve() {
  Serial.println(">> Opening valve...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Valve: OPEN");

  valveServo.write(90);  // open position
}

void closeValve() {
  Serial.println(">> Closing valve...");
  lcd.setCursor(0, 1);
  lcd.print("Valve: CLOSE");
  valveServo.write(0);   // closed position
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  int sensorValue = analogRead(MOISTURE_PIN);
  int moisturePercent = map(sensorValue, dryValue, wetValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Moisture: ");
  Serial.print(moisturePercent);
  Serial.print("% | Temp: ");
  Serial.print(t);
  Serial.print("C | Hum: ");
  Serial.println(h);

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t, 1);
  lcd.print("C H:");
  lcd.print(h, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Moist:");
  lcd.print(moisturePercent);
  lcd.print("% ");
  lcd.print(moisturePercent < 40 ? "DRY" : "WET");

  unsigned long now = millis();

  // === Auto Stir + Valve Logic ===
  if (moisturePercent < moistureThreshold && !stirredRecently) {
  openValve();             // Step 1: Start adding water
  delay(3000);             // Keep valve open for 3s before stirring

  stirCompost();           // Step 2: Stir the compost (6–8 seconds total)

  delay(2000);             // Step 3: Wait a bit for moisture to spread
  closeValve();            // Step 4: Stop water flow

  stirredRecently = true;
  lastStirTime = now;
}


  // Allow next stir after 6 hours
  if (now - lastStirTime > stirInterval) {
    stirredRecently = false;
  }

  delay(3000);
  lcd.clear();
} 