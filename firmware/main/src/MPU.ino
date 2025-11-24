#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// Real thresholds
const int IMPACT_THRESHOLD = 25000;        // ≈3g impact
const int ROTATION_THRESHOLD = 32750;      // sudden rotation spike
const float TILT_ANGLE_THRESHOLD = 45.0;   // fall/tilt angle

void setup() {
  Serial.begin(115200);
  Wire.begin();

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 NOT CONNECTED!");
    while (1);
  }
  Serial.println("MPU6050 Ready...");
}

void loop() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // -------- ACCELEROMETER IMPACT CHECK --------
  if (abs(ax) > IMPACT_THRESHOLD || abs(ay) > IMPACT_THRESHOLD || abs(az) > IMPACT_THRESHOLD) {
    Serial.println("Reason: Impact (Sudden G-force spike detected)");
  }

  // -------- GYROSCOPE ROTATION SPIKE CHECK --------
  if (abs(gx) > ROTATION_THRESHOLD || abs(gy) > ROTATION_THRESHOLD || abs(gz) > ROTATION_THRESHOLD) {
    Serial.println("Reason: Rotation Spike (Sudden angular velocity detected)");
  }

  // -------- TILT/FALL CHECK (ANGLE) --------
  float angleX = atan2(ay, az) * 57.2958;   // convert rad → deg
  float angleY = atan2(ax, az) * 57.2958;

  if (abs(angleX) > TILT_ANGLE_THRESHOLD || abs(angleY) > TILT_ANGLE_THRESHOLD) {
    Serial.println("Reason: Tilt/Fall (Device angle exceeded safe limit)");
  }

  delay(200);
}