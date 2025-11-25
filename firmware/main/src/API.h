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
  bool vibrationDetected,
  float temperature,
  float humidity,
  int coLevel,
  int flameLevel
);

// Get MAC address
String api_getMacAddress();

// Check if connected to WiFi
bool api_isConnected();

#endif
