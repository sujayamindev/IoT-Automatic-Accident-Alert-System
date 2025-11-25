#include "API.h"
#include "SIM.h"
#include "Config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

String deviceMacAddress = "";
WiFiClientSecure secureClient;

bool api_init() {
  Serial.println("\n=== Initializing API Client ===");
  
  // Initialize WiFi mode FIRST to enable MAC address retrieval
  WiFi.mode(WIFI_STA);
  delay(100);  // Small delay to ensure WiFi hardware is ready
  
  // Get MAC address AFTER WiFi mode is set
  deviceMacAddress = WiFi.macAddress();
  Serial.println("Device MAC Address: " + deviceMacAddress);
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    return true;
  } else {
    Serial.println("\nWiFi Connection Failed!");
    return false;
  }
}

String api_getMacAddress() {
  return deviceMacAddress;
}

bool api_isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool api_sendTelemetry(
  double latitude,
  double longitude,
  float speed,
  bool fireDetected,
  bool smokeDetected,
  int16_t accel_x,
  int16_t accel_y,
  int16_t accel_z,
  int16_t gyro_x,
  int16_t gyro_y,
  int16_t gyro_z,
  float temperature,
  float humidity,
  int coLevel,
  int flameLevel,
  String gpsTimestamp
) {
  if (!api_isConnected()) {
    Serial.println("Error: Not connected to WiFi");
    return false;
  }
  
  // Build URL
  String url = String(BACKEND_API_URL) + String(TELEMETRY_ENDPOINT);
  
  // Check if using HTTPS and configure accordingly
  bool useHTTPS = url.startsWith("https://");
  if (useHTTPS) {
    // Configure secure client to skip certificate verification
    secureClient.setInsecure();
  }
  
  // Create JSON payload
  JsonDocument doc;
  doc["deviceId"] = deviceMacAddress;
  doc["lat"] = latitude;
  doc["lng"] = longitude;
  doc["spd"] = speed;
  doc["fire"] = fireDetected ? 1 : 0;
  doc["smoke"] = smokeDetected ? 1 : 0;
  
  // MPU6050 data
  doc["accel_x"] = accel_x;
  doc["accel_y"] = accel_y;
  doc["accel_z"] = accel_z;
  doc["gyro_x"] = gyro_x;
  doc["gyro_y"] = gyro_y;
  doc["gyro_z"] = gyro_z;
  
  // Add extra sensor data (optional fields)
  doc["temp"] = temperature;
  doc["humidity"] = humidity;
  doc["coLevel"] = coLevel;
  doc["flameLevel"] = flameLevel;
  
  // Add GPS timestamp if available
  if (gpsTimestamp.length() > 0) {
    doc["gpsTimestamp"] = gpsTimestamp;
  }
  
  // Optional: Add GSM signal level (0 for WiFi)
  doc["gsm"] = 0;
  
  String jsonPayload;
  serializeJson(doc, jsonPayload);
  
  HTTPClient http;
  if (useHTTPS) {
    http.begin(secureClient, url);  // Use secure client for HTTPS
  } else {
    http.begin(url);  // Regular HTTP connection
  }
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(30000);  // 30 second timeout (for Render cold start)
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);  // Follow HTTP 307 redirects
  
  int httpCode = http.POST(jsonPayload);
  
  if (httpCode > 0) {
    String response = http.getString();
    
    if (httpCode == 200 || httpCode == 201) {
      Serial.println("✓ Telemetry uploaded");
      http.end();
      return true;
    } else if (httpCode == 307 || httpCode == 308 || httpCode == 301 || httpCode == 302) {
      Serial.println("⚠ Redirect - retrying HTTPS...");
      http.end();
      
      // Retry with HTTPS directly
      String httpsUrl = "https://iot-automatic-accident-alert-system.onrender.com" + String(TELEMETRY_ENDPOINT);
      secureClient.setInsecure();
      http.begin(secureClient, httpsUrl);
      http.addHeader("Content-Type", "application/json");
      http.setTimeout(30000);
      
      int retryCode = http.POST(jsonPayload);
      
      if (retryCode == 200 || retryCode == 201) {
        Serial.println("✓ Telemetry uploaded (HTTPS)");
        http.end();
        return true;
      } else if (retryCode == 403) {
        Serial.println("✗ Device not bound");
      }
      http.end();
      return false;
    } else if (httpCode == 403) {
      Serial.println("✗ Device not bound");
    } else if (httpCode == 400) {
      Serial.println("✗ Invalid data");
    } else {
      Serial.print("✗ HTTP ");
      Serial.println(httpCode);
    }
  } else {
    Serial.print("✗ Failed: ");
    Serial.println(http.errorToString(httpCode));
  }
  
  http.end();
  return false;
}

bool api_sendTelemetryViaGPRS(
  double latitude,
  double longitude,
  float speed,
  bool fireDetected,
  bool smokeDetected,
  int16_t accel_x,
  int16_t accel_y,
  int16_t accel_z,
  int16_t gyro_x,
  int16_t gyro_y,
  int16_t gyro_z,
  float temperature,
  float humidity,
  int coLevel,
  int flameLevel,
  String gpsTimestamp
) {
  Serial.println("Sending telemetry via GPRS...");
  
  // Build URL (use HTTP version for GPRS - SIM800L doesn't support HTTPS)
  String url = String(BACKEND_API_URL_HTTP) + String(TELEMETRY_ENDPOINT);
  
  // Create JSON payload (same structure as WiFi)
  JsonDocument doc;
  doc["deviceId"] = deviceMacAddress;
  doc["lat"] = latitude;
  doc["lng"] = longitude;
  doc["spd"] = speed;
  doc["fire"] = fireDetected ? 1 : 0;
  doc["smoke"] = smokeDetected ? 1 : 0;
  
  // MPU6050 data
  doc["accel_x"] = accel_x;
  doc["accel_y"] = accel_y;
  doc["accel_z"] = accel_z;
  doc["gyro_x"] = gyro_x;
  doc["gyro_y"] = gyro_y;
  doc["gyro_z"] = gyro_z;
  
  // Additional sensor data
  doc["temp"] = temperature;
  doc["humidity"] = humidity;
  doc["coLevel"] = coLevel;
  doc["flameLevel"] = flameLevel;
  
  // GPS timestamp
  if (gpsTimestamp.length() > 0) {
    doc["gpsTimestamp"] = gpsTimestamp;
  }
  
  // GSM signal level (could read actual signal from SIM module)
  doc["gsm"] = 1;  // Indicate GPRS mode
  
  String jsonPayload;
  serializeJson(doc, jsonPayload);
  
  // Send via SIM module
  return sim_sendHTTPPost(url, jsonPayload);
}
