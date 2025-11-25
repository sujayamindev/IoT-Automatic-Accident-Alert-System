#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// -------------------- WIFI CONFIGURATION --------------------
constexpr const char* WIFI_SSID = "1234";
constexpr const char* WIFI_PASSWORD = "mindev12394";

// -------------------- BACKEND API CONFIGURATION --------------------
constexpr const char* BACKEND_API_URL = "http://10.161.234.199:5000";
constexpr const char* TELEMETRY_ENDPOINT = "/api/device/uploadTelemetry";
constexpr const int UPLOAD_INTERVAL_MS = 10000;  // 10 seconds (fixed interval)

// -------------------- SENSOR PINS --------------------
constexpr const int DHT11_PIN = 4;        // DHT11 temperature/humidity sensor
constexpr const int MQ7_PIN = 34;         // MQ-7 CO gas sensor (analog)
constexpr const int FLAME_PIN = 35;       // Flame sensor (analog)

// -------------------- SENSOR THRESHOLDS --------------------
constexpr const int FLAME_THRESHOLD = 1000;  // Lower values indicate fire
constexpr const int MQ7_THRESHOLD = 1500;    // Higher values indicate CO presence

// -------------------- SMS GATEWAY CONFIGURATION --------------------
constexpr const char* SMS_GATEWAY_IP = "10.64.62.82";
constexpr const int SMS_GATEWAY_PORT = 8082;
constexpr const char* SMS_GATEWAY_TOKEN = "dc916541-d7ec-484e-a405-be9daadb9f71";

// -------------------- PHONE NUMBERS --------------------
// Phone numbers for both WiFi SMS Gateway and GSM Module
inline const char* const PHONE_NUMBERS[] = {"+94717016900"};

// -------------------- BLUETOOTH CONFIGURATION --------------------
constexpr const char* BLUETOOTH_DEVICE_NAME = "ESP32_Accident_Alert";

#endif
