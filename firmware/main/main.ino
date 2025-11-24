#include "src/MPU.h"
#include "src/GPS.h"
#include "src/SIM.h"
#include "src/SMS.h"
#include "src/Config.h"
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

bool accidentDetected = false;
bool simAvailable = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Accident Alert System Starting...");
  
  // Initialize Bluetooth Serial
  SerialBT.begin(BLUETOOTH_DEVICE_NAME);
  Serial.println("Bluetooth Serial initialized - Device Name: ESP32_Accident_Alert");
  SerialBT.println("Accident Alert System - Bluetooth Connected");
  
  // Initialize sensors
  mpu_init();
  gps_init();
  
  // Try to initialize GSM module
  Serial.println("Initializing GSM module...");
  SerialBT.println("Initializing GSM module...");
  sim_init();
  
  // Check if SIM800L is available (you can implement a check in SIM.cpp)
  // For now, we'll try WiFi as fallback
  Serial.println("Initializing WiFi SMS Gateway as fallback...");
  SerialBT.println("Initializing WiFi SMS Gateway...");
  sms_init();
  
  // Determine which module is available
  simAvailable = true;  // Set based on actual SIM module check
  
  Serial.println("System Ready!");
  SerialBT.println("System Ready!");
}

void loop() {
  // Update GPS data continuously
  gps_update();
  
  // Check for impact or tilt
  bool impact = mpu_detectImpact();
  bool tilt = mpu_detectTilt();
  
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
  
  delay(100);  // Small delay for stability
}
