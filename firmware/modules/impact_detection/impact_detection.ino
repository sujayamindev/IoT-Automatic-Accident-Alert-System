// Standalone Impact Detection Module
// Tests MPU6050 sensor for crash and rollover detection
// No GPS or Firebase required

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// Thresholds
float impactThreshold = 22.0;    // m/s²  (≈ 2.2g impact)
float tiltThreshold   = 55.0;    // degrees (vehicle fall angle)

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);   // SDA=21, SCL=22 for ESP32

  Serial.println("Initializing MPU6050...");
  if (!mpu.begin(0x68)) {   // use 0x69 if your scanner shows that
    Serial.println("❌ MPU6050 not found!");
    while (1);
  }

  Serial.println("✅ MPU6050 Connected!");
  delay(1500);
}

void loop() {

  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Calculate acceleration magnitude
  float totalAcc = sqrt(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y +
    accel.acceleration.z * accel.acceleration.z
  );

  // Calculate tilt angles
  float roll  = atan2(accel.acceleration.y, accel.acceleration.z) * 57.3;
  float pitch = atan2(-accel.acceleration.x,
                sqrt(accel.acceleration.y * accel.acceleration.y +
                     accel.acceleration.z * accel.acceleration.z)) * 57.3;

  // Print readings
  Serial.print("Acc: ");
  Serial.print(totalAcc);
  Serial.print("  | Roll: ");
  Serial.print(roll);
  Serial.print("  | Pitch: ");
  Serial.println(pitch);

  // Detect impact and fall
  bool impactDetected = (totalAcc > impactThreshold);
  bool fallDetected   = (abs(roll) > tiltThreshold || abs(pitch) > tiltThreshold);

  if (impactDetected) {
    Serial.println("⚠ IMPACT DETECTED!");
  }

  if (fallDetected) {
    Serial.println("⚠ FALL / TILT DETECTED!");
  }

  // Accident = impact OR fall
  if (impactDetected || fallDetected) {
    Serial.println("🚨 ACCIDENT DETECTED 🚨");
    Serial.println("--------------------------------");
    delay(3000);  // avoid repeated alerts
  }

  delay(200);
}
