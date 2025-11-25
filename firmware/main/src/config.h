#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// -------------------- WIFI CONFIGURATION --------------------
constexpr const char* WIFI_SSID = "1234";
constexpr const char* WIFI_PASSWORD = "mindev12394";

// -------------------- BACKEND API CONFIGURATION --------------------
// Production URLs for Render deployment
constexpr const char* BACKEND_API_URL = "https://iot-automatic-accident-alert-system.onrender.com";
constexpr const char* BACKEND_API_URL_HTTP = "http://iot-automatic-accident-alert-system.onrender.com";  // For GPRS (no HTTPS support)
constexpr const char* TELEMETRY_ENDPOINT = "/api/device/uploadTelemetry";
constexpr const int UPLOAD_INTERVAL_MS = 3000;  // 3 seconds (fixed interval)

// Connection mode: false = WiFi first (GPRS fallback), true = GPRS first (WiFi fallback)
constexpr const bool GPRS_PRIMARY_MODE = false;

// -------------------- GPRS CONFIGURATION --------------------
constexpr const char* GPRS_APN = "dialogbb";           // Dialog SIM APN
constexpr const char* GPRS_USERNAME = "";              // Usually empty for Dialog
constexpr const char* GPRS_PASSWORD = "";              // Usually empty for Dialog
constexpr const int GPRS_TIMEOUT_MS = 30000;           // 30 second timeout for HTTP requests

// -------------------- SENSOR PINS --------------------
constexpr const int DHT11_PIN = 4;        // DHT11 temperature/humidity sensor
constexpr const int MQ7_PIN = 34;         // MQ-7 CO gas sensor (analog)
constexpr const int FLAME_PIN = 35;       // Flame sensor (analog)

// -------------------- SENSOR THRESHOLDS --------------------
constexpr const int FLAME_THRESHOLD = 1000;     // Lower values indicate fire (analog mode)
constexpr const int MQ7_SMOKE_THRESHOLD = 250;  // Smoothed MQ7 value indicating smoke
constexpr const float EXTREME_TEMP_THRESHOLD = 50.0;  // Temperature in Celsius indicating extreme heat

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
