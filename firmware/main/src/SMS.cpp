#include "SMS.h"
#include "Config.h"
#include <WiFi.h>
#include <HTTPClient.h>

// JSON escape helper function
String jsonEscape(String str) {
  String escaped = "";
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (c == '"') {
      escaped += "\\\"";
    } else if (c == '\\') {
      escaped += "\\\\";
    } else if (c == '\n') {
      escaped += "\\n";
    } else if (c == '\r') {
      escaped += "\\r";
    } else if (c == '\t') {
      escaped += "\\t";
    } else {
      escaped += c;
    }
  }
  return escaped;
}

void sms_init() {
  Serial.println("WiFi SMS Gateway Initializing...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("SMS Gateway Ready!");
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

bool sms_isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void sms_sendAccidentAlert(String mapsLink) {
  if (!sms_isConnected()) {
    Serial.println("WiFi disconnected! Attempting to reconnect...");
    WiFi.reconnect();
    delay(5000);
    
    if (!sms_isConnected()) {
      Serial.println("WiFi reconnection failed!");
      return;
    }
  }
  
  String message = "ACCIDENT ALERT!\nVehicle has stopped.\nLocation: " + mapsLink;
  
  // Send SMS to all numbers
  int numPhones = sizeof(PHONE_NUMBERS) / sizeof(PHONE_NUMBERS[0]);
  for (int i = 0; i < numPhones; i++) {
    Serial.print("Sending SMS via WiFi to ");
    Serial.println(PHONE_NUMBERS[i]);
    
    HTTPClient http;
    
    // Traccar SMS Gateway API endpoint
    String url = "http://" + String(SMS_GATEWAY_IP) + ":" + String(SMS_GATEWAY_PORT) + "/";
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", SMS_GATEWAY_TOKEN);
    
    // Build JSON payload
    String jsonPayload = "{\"to\":\"" + String(PHONE_NUMBERS[i]) + "\",\"message\":\"" + jsonEscape(message) + "\"}";
    
    int httpCode = http.POST(jsonPayload);
    
    if (httpCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpCode);
      
      String response = http.getString();
      Serial.println("Response: " + response);
      
      if (httpCode == 200) {
        Serial.println("SMS sent successfully!");
      } else {
        Serial.println("SMS failed!");
      }
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(http.errorToString(httpCode));
    }
    
    http.end();
    delay(1000);  // Wait between messages
  }
}
