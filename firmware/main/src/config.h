#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// -------------------- WIFI CONFIGURATION --------------------
constexpr const char* WIFI_SSID = "1234";
constexpr const char* WIFI_PASSWORD = "mindev12394";

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
