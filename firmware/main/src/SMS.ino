// ESP32 + Traccar SMS Gateway (WiFi)

#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "1234";
const char* password = "mindev12394";

// Traccar SMS Gateway settings
String gatewayIP = "10.64.62.82";  // Your Android phone IP address
int gatewayPort = 8082;              // Default Traccar gateway port

// Gateway authentication (check in Traccar app settings)
String gatewayToken = "dc916541-d7ec-484e-a405-be9daadb9f71";  // If token-based auth is enabled

// Phone numbers to send SMS
String phoneNumbers[] = {"+94717016900", "+94769696558", "+94722115153"};
int numPhones = 3;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
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
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

void loop() {
  // Example: Send SMS when needed
  String message = "ACCIDENT ALERT!\nVehicle has stopped.\nLocation: https://maps.google.com/maps?q=6.927079,79.861244";
  
  if (WiFi.status() == WL_CONNECTED) {
    // Send SMS to all numbers
    for (int i = 0; i < numPhones; i++) {
      Serial.print("Sending SMS to ");
      Serial.println(phoneNumbers[i]);
      
      HTTPClient http;
      
      // Traccar SMS Gateway API endpoint
      String url = "http://" + gatewayIP + ":" + String(gatewayPort) + "/";
      
      Serial.println("URL: " + url);
      
      http.begin(url);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", gatewayToken);  // Use API key as Authorization header
      
      // Build JSON payload
      String jsonPayload = "{\"to\":\"" + phoneNumbers[i] + "\",\"message\":\"" + jsonEscape(message) + "\"}";
      
      Serial.println("JSON payload: " + jsonPayload);
      
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
  } else {
    Serial.println("WiFi disconnected!");
    WiFi.reconnect();
  }
  
  // Wait before next batch (remove in real implementation)
  Serial.println("Waiting 60 seconds...");
  delay(60000);
}

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
