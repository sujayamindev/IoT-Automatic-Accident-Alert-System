#ifndef API_H
#define API_H

#include <Arduino.h>

// Initialize API client (connects to WiFi)
bool api_init();

// Send telemetry data to backend
bool api_sendTelemetry(
  double latitude,
  double longitude,
  float speed,
  bool fireDetected,
  bool smokeDetected,
  bool hasImpact,
  int16_t accel_x,
  int16_t accel_y,
  int16_t accel_z,
  int16_t gyro_x,
  int16_t gyro_y,
  int16_t gyro_z,
  float temperature,
  float humidity,
  int coLevel,
  String gpsTimestamp
);

// Get MAC address
String api_getMacAddress();

// Check if connected to WiFi
bool api_isConnected();

// Send telemetry data via GPRS (same parameters as WiFi version)
bool api_sendTelemetryViaGPRS(
  double latitude,
  double longitude,
  float speed,
  bool fireDetected,
  bool smokeDetected,
  bool hasImpact,
  int16_t accel_x,
  int16_t accel_y,
  int16_t accel_z,
  int16_t gyro_x,
  int16_t gyro_y,
  int16_t gyro_z,
  float temperature,
  float humidity,
  int coLevel,
  String gpsTimestamp
);

#endif
