#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// -------------------- WIFI CONFIGURATION --------------------
constexpr const char* WIFI_SSID = "YOUR_WIFI_SSID";
constexpr const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// -------------------- BACKEND API CONFIGURATION --------------------
// Production URLs for Render deployment
constexpr const char* BACKEND_API_URL = "https://your-backend-url.onrender.com";
constexpr const char* BACKEND_API_URL_HTTP = "http://your-backend-url.onrender.com";  // For GPRS (no HTTPS support)
constexpr const char* TELEMETRY_ENDPOINT = "/api/device/uploadTelemetry";
constexpr const int UPLOAD_INTERVAL_MS = 1000;  // 1 second (fixed interval)

// Connection mode: false = WiFi first (GPRS fallback), true = GPRS first (WiFi fallback)
constexpr const bool GPRS_PRIMARY_MODE = false;

// -------------------- GPRS CONFIGURATION --------------------
constexpr const char* GPRS_APN = "your-mobile-apn";           // Your mobile carrier APN (e.g., "dialogbb" for Dialog)
constexpr const char* GPRS_USERNAME = "";              // Usually empty for most carriers
constexpr const char* GPRS_PASSWORD = "";              // Usually empty for most carriers
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
// Note: If using WiFi SMS gateway, configure your local SMS gateway server
constexpr const char* SMS_GATEWAY_IP = "192.168.1.100";  // Your SMS gateway server IP
constexpr const int SMS_GATEWAY_PORT = 8082;
constexpr const char* SMS_GATEWAY_TOKEN = "your-sms-gateway-token";

// -------------------- PHONE NUMBERS --------------------
// Phone numbers for both WiFi SMS Gateway and GSM Module
// Format: International format with country code (e.g., "+94771234567" for Sri Lanka)
inline const char* const PHONE_NUMBERS[] = {"+94771234567", "+94779876543", "+94112345678"};

#endif
