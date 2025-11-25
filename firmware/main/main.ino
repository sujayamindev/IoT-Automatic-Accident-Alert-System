#include "src/MPU.h"
#include "src/GPS.h"
#include "src/SIM.h"
#include "src/SMS.h"
#include "src/Config.h"
#include "src/API.h"
#include "src/Sensors.h"
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

bool accidentDetected = false;
bool simAvailable = false;
unsigned long lastTelemetryUpload = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=================================");
  Serial.println("Accident Alert System Starting...");
  Serial.println("=================================");
  
  // Initialize Bluetooth Serial
  SerialBT.begin(BLUETOOTH_DEVICE_NAME);
  Serial.println("Bluetooth Serial initialized - Device Name: ESP32_Accident_Alert");
  SerialBT.println("Accident Alert System - Bluetooth Connected");
  
  // Initialize WiFi and API client (for telemetry)
  Serial.println("\n--- WiFi & API Initialization ---");
  if (api_init()) {
    Serial.println("✓ API Client ready");
    Serial.println("Device MAC: " + api_getMacAddress());
    SerialBT.println("WiFi Connected - Device MAC: " + api_getMacAddress());
  } else {
    Serial.println("✗ API Client failed - will retry in loop");
    SerialBT.println("WiFi Connection Failed - will retry");
  }
  
  // Initialize core sensors
  Serial.println("\n--- Core Sensors Initialization ---");
  mpu_init();
  gps_init();
  
  // Initialize additional sensors (DHT11, MQ-7, Flame)
  Serial.println("\n--- Additional Sensors Initialization ---");
  sensors_init();
  
  // Try to initialize GSM module (for SMS backup)
  Serial.println("\n--- GSM Module Initialization ---");
  SerialBT.println("Initializing GSM module...");
  sim_init();
  
  // Initialize WiFi SMS Gateway as fallback
  Serial.println("\n--- SMS Gateway Initialization ---");
  SerialBT.println("Initializing WiFi SMS Gateway...");
  sms_init();
  
  // Determine which module is available
  simAvailable = true;  // Set based on actual SIM module check
  
  Serial.println("\n=================================");
  Serial.println("System Ready!");
  Serial.println("Telemetry upload every " + String(UPLOAD_INTERVAL_MS / 1000) + " seconds");
  Serial.println("=================================\n");
  SerialBT.println("System Ready!");
}

void loop() {
  // Update GPS data continuously
  gps_update();
  
  // Check for impact or tilt
  bool impact = mpu_detectImpact();
  bool tilt = mpu_detectTilt();
  
  // Read additional sensors
  bool fireDetected = sensors_isFireDetected();
  bool smokeDetected = sensors_isCODetected();
  
  // If accident detected and not already processed
  if ((impact || tilt) && !accidentDetected) {
    accidentDetected = true;
    Serial.println("\n---------------------------------");
    Serial.println("ACCIDENT DETECTED!");
    Serial.println("---------------------------------");
    SerialBT.println("\n---------------------------------");
    SerialBT.println("ACCIDENT DETECTED!");
    SerialBT.println("---------------------------------");
    
    // Wait a moment for vehicle to come to a stop
    delay(100);
  }
  
  // If accident was detected, check if vehicle has stopped
  if (accidentDetected && gps_isStopped()) {
    Serial.println("\n*** VEHICLE STOPPED - SENDING LOCATION ***");
    SerialBT.println("\n*** VEHICLE STOPPED - SENDING LOCATION ***");
    
    // Get and print Google Maps link
    String mapsLink = gps_getGoogleMapsLink();
    double lat = gps_getLatitude();
    double lng = gps_getLongitude();
    
    Serial.println("Google Maps Link: " + mapsLink);
    Serial.print("Latitude: ");
    Serial.println(lat, 6);
    Serial.print("Longitude: ");
    Serial.println(lng, 6);
    
    SerialBT.println("Google Maps Link: " + mapsLink);
    SerialBT.print("Latitude: ");
    SerialBT.println(lat, 6);
    SerialBT.print("Longitude: ");
    SerialBT.println(lng, 6);
    
    // Try GSM first, fallback to WiFi only if GSM fails
    Serial.println("Attempting to send via GSM...");
    SerialBT.println("Attempting to send via GSM...");
    bool gsmSuccess = sim_sendAccidentAlert(mapsLink);
    
    // Use WiFi SMS if GSM failed
    if (!gsmSuccess) {
      Serial.println("GSM failed, using WiFi SMS Gateway...");
      SerialBT.println("GSM failed, using WiFi SMS Gateway...");
      sms_sendAccidentAlert(mapsLink);
    } else {
      Serial.println("SMS sent via GSM successfully!");
      SerialBT.println("SMS sent via GSM successfully!");
    }
    
    Serial.println("---------------------------------\n");
    SerialBT.println("---------------------------------\n");
    
    // Reset flag (or keep it true if you want to send only once)
    accidentDetected = false;
  }
  
  // ===== TELEMETRY UPLOAD TO BACKEND (NEW) =====
  unsigned long currentMillis = millis();
  if (currentMillis - lastTelemetryUpload >= UPLOAD_INTERVAL_MS) {
    lastTelemetryUpload = currentMillis;
    
    // Reconnect WiFi if disconnected
    if (!api_isConnected()) {
      Serial.println("WiFi disconnected, reconnecting...");
      api_init();
    }
    
    if (api_isConnected()) {
      // Gather all sensor data
      double lat = gps_getLatitude();
      double lng = gps_getLongitude();
      float speed = gps_getSpeed();
      bool vibration = impact || tilt;  // Any detected impact/tilt
      float temperature = sensors_getTemperature();
      float humidity = sensors_getHumidity();
      int coLevel = sensors_getCOLevel();
      int flameLevel = sensors_getFlameLevel();
      
      // Send telemetry to backend
      bool success = api_sendTelemetry(
        lat, lng, speed,
        fireDetected, smokeDetected, vibration,
        temperature, humidity, coLevel, flameLevel
      );
      
      if (success) {
        SerialBT.println("✓ Telemetry uploaded");
      } else {
        Serial.println("✗ Telemetry upload failed");
        SerialBT.println("✗ Telemetry upload failed");
      }
    }
  }
  
  delay(100);  // Small delay for stability
}
