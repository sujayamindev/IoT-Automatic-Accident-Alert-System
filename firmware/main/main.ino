#include "src/MPU.h"
#include "src/GPS.h"

bool accidentDetected = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Accident Alert System Starting...");
  
  // Initialize sensors
  mpu_init();
  gps_init();
  
  Serial.println("System Ready!");
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
    
    // Wait a moment for vehicle to come to a stop
    delay(100);
  }
  
  // If accident was detected, check if vehicle has stopped
  if (accidentDetected && gps_isStopped()) {
    Serial.println("\n*** VEHICLE STOPPED - SENDING LOCATION ***");
    
    // Get and print Google Maps link
    String mapsLink = gps_getGoogleMapsLink();
    Serial.println("Google Maps Link: " + mapsLink);
    
    Serial.print("Latitude: ");
    Serial.println(gps_getLatitude(), 6);
    Serial.print("Longitude: ");
    Serial.println(gps_getLongitude(), 6);
    
    Serial.println("---------------------------------\n");
    
    // Reset flag (or keep it true if you want to send only once)
    accidentDetected = false;
  }
  
  delay(100);  // Small delay for stability
}
