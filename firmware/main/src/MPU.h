#ifndef MPU_H
#define MPU_H

#include <Wire.h>
#include <MPU6050.h>

// Thresholds
const int IMPACT_THRESHOLD = 25000;        // ≈3g impact
const int ROTATION_THRESHOLD = 32750;      // sudden rotation spike
const float TILT_ANGLE_THRESHOLD = 45.0;   // fall/tilt angle

// Function declarations
void mpu_init();
bool mpu_detectImpact();
bool mpu_detectTilt();
void mpu_getRawData(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);

#endif
