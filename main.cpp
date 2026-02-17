
// #include <Arduino.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <LiquidCrystal_I2C.h>


const int gasSensor = A0;
const int GREEN_LED = 2;
const int RED_LED = 4;
const int BUZZER = 7;


MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

struct mpuData {
  byte X;
  byte Y;
  byte Z;
};

mpuData data;

LiquidCrystal_I2C lcd(0x27, 16, 2);


void setup()
{
  Serial.begin(9600);
  Serial.println("Serial started");
  Wire.begin();
  Serial.println("Wire started");
  mpu.initialize();
  Serial.println("MPU initialized");
  Serial.println("Hello World");
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas/EQ Detector");
  delay(1000);
}

int gyroThreshold = 500;
const int requiredConsecutive = 5;
int consecutiveCount = 0;
bool earthquakeActive = false;

void loop() {
  const char* levelMeanings[] = {"SAFE", "LOW", "MODERATE", "HIGH", "DANGER"};
  int gasValue = analogRead(gasSensor);
  int level;

  if (gasValue <= 204) level = 0;
  else if (gasValue <= 409) level = 1;
  else if (gasValue <= 614) level = 2;
  else if (gasValue <= 819) level = 3;
  else level = 4;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  data.X = map(ax, -17000, 17000, 0, 255 );
  data.Y = map(ay, -17000, 17000, 0, 255);
  data.Z = map(az, -17000, 17000, 0, 255);
  delay(500);
  Serial.println("==============================");
  Serial.println("[Gas Sensor Readings]");
  Serial.print("Raw: ");
  Serial.print(gasValue);
  Serial.print("  Level: ");
  Serial.print(level);
  Serial.print("  Meaning: ");
  Serial.println(levelMeanings[level]);
  Serial.println("[MPU6050 Sensor Readings]");
  Serial.print("Gyro (deg/s):  X=");
  Serial.print(gx);
  Serial.print("  Y=");
  Serial.print(gy);
  Serial.print("  Z=");
  Serial.println(gz);

  Serial.print("Accel (mapped): X=");
  Serial.print(data.X);
  Serial.print("  Y=");
  Serial.print(data.Y);
  Serial.print("  Z=");
  Serial.println(data.Z);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Level: ");
  lcd.print(level);
  lcd.setCursor(0, 1);
  lcd.print(levelMeanings[level]);

  bool overThreshold = (abs(gx) > gyroThreshold) || (abs(gy) > gyroThreshold) || (abs(gz) > gyroThreshold);
  if (overThreshold) {
    consecutiveCount++;
    Serial.print("[!] Gyro over threshold! Consecutive: ");
    Serial.println(consecutiveCount);
    if (!earthquakeActive && consecutiveCount >= requiredConsecutive) {
      earthquakeActive = true;
      Serial.println("*** EARTHQUAKE DETECTED! ***");
    }
  } else {
    if (consecutiveCount > 0) {
      Serial.println("[i] Gyro below threshold, resetting count.");
    }
    consecutiveCount = 0;
    if (earthquakeActive) {
      earthquakeActive = false;
      Serial.println("--- No earthquake. ---");
    }
  }

  if (earthquakeActive || level >= 3) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, HIGH);
    if (earthquakeActive && level >= 3) {
      Serial.println("[LED] RED ON (Earthquake & Gas Danger), GREEN OFF");
    } else if (earthquakeActive) {
      Serial.println("[LED] RED ON (Earthquake), GREEN OFF");
    } else {
      Serial.println("[LED] RED ON (Gas Danger), GREEN OFF");
    }
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER, LOW);
    Serial.println("[LED] RED OFF, GREEN ON");
  }
  Serial.println();
}
