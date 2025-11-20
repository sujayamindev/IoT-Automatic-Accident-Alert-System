#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// Simple accident detection threshold
float IMPACT_THRESHOLD = 1.0;  // g-force threshold

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Initializing MPU6050...");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050. Check wiring!");
    while (1) delay(10);
  }

  // Configure accelerometer range (keep it simple)
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 OK!");
}

void loop() {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Calculate total acceleration (overall g-force)
  float ax = accel.acceleration.x / 9.81;
  float ay = accel.acceleration.y / 9.81;
  float az = accel.acceleration.z / 9.81;

  float totalG = sqrt(ax * ax + ay * ay + az * az);

  Serial.print("G-force: ");
  Serial.println(totalG, 2);

  // Simple impact detection
  if (totalG > IMPACT_THRESHOLD) {
    Serial.println("⚠️  IMPACT DETECTED! Possible Accident!");
  }

  delay(1000);
}
