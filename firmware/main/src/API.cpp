#include "API.h"
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
  bool vibrationDetected,
  float temperature,
  float humidity,
  int coLevel,
  int flameLevel
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
  StaticJsonDocument<512> doc;
  doc["deviceId"] = deviceMacAddress;
  doc["lat"] = latitude;
  doc["lng"] = longitude;
  doc["spd"] = speed;
  doc["fire"] = fireDetected ? 1 : 0;
  doc["smoke"] = smokeDetected ? 1 : 0;
  doc["vib"] = vibrationDetected ? 1 : 0;
  
  // Add extra sensor data (optional fields)
  doc["temp"] = temperature;
  doc["humidity"] = humidity;
  doc["coLevel"] = coLevel;
  doc["flameLevel"] = flameLevel;
  
  // Optional: Add GSM signal level (0 for WiFi)
  doc["gsm"] = 0;
  
  // Optional: Add battery percentage (not implemented yet)
  // doc["bat"] = batteryPercentage;
  
  String jsonPayload;
  serializeJson(doc, jsonPayload);
  
  Serial.println("\n--- Sending Telemetry ---");
  Serial.println("URL: " + url);
  Serial.println("Payload: " + jsonPayload);
  
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
  
  Serial.print("HTTP Response Code: ");
  Serial.println(httpCode);
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Response: " + response);
    
    if (httpCode == 200 || httpCode == 201) {
      Serial.println("✓ Telemetry sent successfully!");
      http.end();
      return true;
    } else if (httpCode == 307 || httpCode == 308 || httpCode == 301 || httpCode == 302) {
      Serial.println("⚠ Redirect detected - trying HTTPS directly...");
      http.end();
      
      // Retry with HTTPS directly
      String httpsUrl = "https://iot-automatic-accident-alert-system.onrender.com" + String(TELEMETRY_ENDPOINT);
      secureClient.setInsecure();
      http.begin(secureClient, httpsUrl);
      http.addHeader("Content-Type", "application/json");
      http.setTimeout(30000);
      
      int retryCode = http.POST(jsonPayload);
      Serial.print("HTTPS Retry Response Code: ");
      Serial.println(retryCode);
      
      if (retryCode == 200 || retryCode == 201) {
        Serial.println("✓ Telemetry sent successfully via HTTPS!");
        http.end();
        return true;
      } else if (retryCode == 403) {
        Serial.println("✗ Device not bound to user! Bind device in mobile app first.");
      } else {
        response = http.getString();
        Serial.println("Response: " + response);
      }
      http.end();
      return false;
    } else if (httpCode == 403) {
      Serial.println("✗ Device not bound to user! Bind device in mobile app first.");
    } else if (httpCode == 400) {
      Serial.println("✗ Invalid data format! Check payload structure.");
    } else {
      Serial.println("✗ Server error");
    }
  } else {
    Serial.print("✗ Connection failed: ");
    Serial.println(http.errorToString(httpCode));
    
    // Specific error handling
    if (httpCode == -1) {
      Serial.println("Hint: Backend may be sleeping (Render free tier takes 30s to wake up)");
      Serial.println("Or check backend URL is correct and accessible");
    } else if (httpCode == -11) {
      Serial.println("Hint: Check WiFi connection and DNS resolution");
    }
  }
  
  http.end();
  return false;
}
