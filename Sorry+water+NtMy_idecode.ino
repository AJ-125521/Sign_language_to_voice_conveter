#include "BluetoothSerial.h"
#include <Wire.h>
#include <MPU6050.h>

BluetoothSerial SerialBT;
MPU6050 mpu;

// --- PINOUT ---
const int thumbPin  = 35; 
const int indexPin  = 33;
const int middlePin = 34; 
const int pinkyPin  = 32;

// --- CALIBRATED THRESHOLDS ---
const int THUMB_BENT_LIMIT = 85; 
const int MID_BENT_LIMIT   = 2420; 
const int INDEX_BENT_LIMIT = 70; 
const int PINKY_BENT_LIMIT = 70; 


const int THUMB_STRAIGHT_reg = 71; 
const int INDEX_STRAIGHT_reg = 71; 
const int MID_STRAIGHT_reg   = 2425; 
//const float TILT_THRESHOLD   = -0.90; 

// --- MOTION, STABILITY & TIMING ---
int prevThumb = 0;
int prevMiddle = 0;
int16_t prevY = 0, prevZ = 0; // For circular motion
int circlePoints = 0; 
const int MOTION_SENSITIVITY = 2000; // Intensity of the rub

const int NOISE_FLOOR = 6; 
unsigned long lastTriggerTime = 0;
const int COOLDOWN = 3000; 

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ISL_Glove_Final");
  
  Wire.begin();
  // Fixed address for your MPU6880
  mpu.initialize();

  pinMode(thumbPin,  INPUT);
  pinMode(indexPin,  INPUT);
  pinMode(middlePin, INPUT);
  pinMode(pinkyPin,  INPUT);

  prevThumb = analogRead(thumbPin);
  prevMiddle = analogRead(middlePin);
  
  Serial.println("--- GLOVE READY: SORRY (CIRCULAR) & NICE TO MEET YOU ---");
}

void loop() {
  // 1. Read Flex Sensors
  int t = analogRead(thumbPin);
  int i = analogRead(indexPin);
  int m = analogRead(middlePin);
  int p = analogRead(pinkyPin);

  // 2. Read MPU6880 Accel and Gyro Data
  int16_t ax_raw, ay_raw, az_raw;
  mpu.getAcceleration(&ax_raw, &ay_raw, &az_raw);
  
  float accX = ax_raw / 16384.0;
  float accY = ay_raw / 16384.0;

  // 3. Calculate Stability and Movement Deltas
  int tStability = abs(t - prevThumb);
  int mStability = abs(m - prevMiddle);
  int deltaY = abs(ay_raw - prevY);
  int deltaZ = abs(az_raw - prevZ);

  // --- GESTURE 1: SORRY (Fist + Circular Motion) ---
  if (t < THUMB_BENT_LIMIT && m < MID_BENT_LIMIT && i <= INDEX_BENT_LIMIT) {
    
    // Detect circular/rubbing motion in Y and Z axes
    if (deltaY > MOTION_SENSITIVITY && deltaZ > MOTION_SENSITIVITY) {
      circlePoints++;
    }

    // Trigger when the circular motion is confirmed
    if (circlePoints >= 5 && (millis() - lastTriggerTime > COOLDOWN)) {
      Serial.println(">>> DETECTED: SORRY (ISL)");
      SerialBT.println("SORRY");
      circlePoints = 0;
      lastTriggerTime = millis();
    }
  }

  // --- GESTURE 2: NICE TO MEET YOU (Straight + 90° Tilt) ---
  else if (accX <= -0.80 && accX >=-1.0 &&
           t >= THUMB_STRAIGHT_reg && 
           i >= INDEX_STRAIGHT_reg && 
           m >= MID_STRAIGHT_reg && 
           (millis() - lastTriggerTime > COOLDOWN)) {
    
    if (tStability < NOISE_FLOOR && mStability < NOISE_FLOOR) {
      Serial.println(">>> DETECTED: NICE TO MEET YOU (ISL)");
      SerialBT.println("NICE TO MEET YOU");
      lastTriggerTime = millis();
    }
  }
  else if (accY <= -0.80 && accY >= -1.00 &&
      t < THUMB_BENT_LIMIT && 
      i >= INDEX_STRAIGHT_reg && 
      m > MID_STRAIGHT_reg && 
      p < PINKY_BENT_LIMIT) {
    
    if (millis() - lastTriggerTime > COOLDOWN) {
      Serial.println(">>> WATER DETECTED <<<");
      SerialBT.println("WATER");
      lastTriggerTime = millis();
    }
  }


  // Reset circlePoints if motion stops for more than 500ms
  static unsigned long lastMove = 0;
  if (deltaY > MOTION_SENSITIVITY || deltaZ > MOTION_SENSITIVITY) lastMove = millis();
  if (millis() - lastMove > 500) circlePoints = 0;

  // Debugging Data
  Serial.print("T: "); Serial.print(t);
  Serial.print(" M: "); Serial.print(m);
  Serial.print(" AccX: "); Serial.print(accX);
  Serial.print(" Circle: "); Serial.println(circlePoints);
  Serial.print(" AccY :"); Serial.println(accY);
  Serial.print("i: "); Serial.print(i);
  Serial.print("p: "); Serial.print(p);

  // Update previous values for next loop
  prevThumb = t;
  prevMiddle = m;
  prevY = ay_raw;
  prevZ = az_raw;
  delay(150); 
}
