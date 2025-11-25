#include "MPU.h"

MPU6050 mpu;

void mpu_init() {
  Wire.begin();
  mpu.initialize();
  
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 NOT CONNECTED!");
    while (1);
  }
  Serial.println("MPU6050 Ready...");
}

bool mpu_detectImpact() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  // Check for sudden G-force spike (impact)
  if (abs(ax) > IMPACT_THRESHOLD || abs(ay) > IMPACT_THRESHOLD || abs(az) > IMPACT_THRESHOLD) {
    Serial.println("Impact detected: Sudden G-force spike");
    return true;
  }
  
  // Check for sudden rotation spike
  if (abs(gx) > ROTATION_THRESHOLD || abs(gy) > ROTATION_THRESHOLD || abs(gz) > ROTATION_THRESHOLD) {
    Serial.println("Impact detected: Sudden angular velocity");
    return true;
  }
  
  return false;
}

bool mpu_detectTilt() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  // Calculate tilt angles
  float angleX = atan2(ay, az) * 57.2958;  // convert rad → deg
  float angleY = atan2(ax, az) * 57.2958;
  
  if (abs(angleX) > TILT_ANGLE_THRESHOLD || abs(angleY) > TILT_ANGLE_THRESHOLD) {
    Serial.println("Tilt detected: Device angle exceeded safe limit");
    return true;
  }
  
  return false;
}

void mpu_getRawData(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
  mpu.getMotion6(ax, ay, az, gx, gy, gz);
}
