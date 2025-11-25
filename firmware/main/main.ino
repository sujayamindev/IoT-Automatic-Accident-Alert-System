#include "src/MPU.h"
#include "src/GPS.h"
#include "src/SIM.h"
#include "src/SMS.h"
#include "src/Config.h"
#include "src/API.h"
#include "src/Sensors.h"

bool accidentDetected = false;
bool simAvailable = false;
unsigned long lastTelemetryUpload = 0;

// SMS alert flags to prevent duplicate messages
bool fireSMSSent = false;
bool smokeSMSSent = false;
bool extremeTempSMSSent = false;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=================================");
  Serial.println("Accident Alert System Starting...");
  Serial.println("=================================");
  
  // Initialize WiFi and API client (for telemetry)
  Serial.println("\n--- WiFi & API Initialization ---");
  if (api_init()) {
    Serial.println("✅ API Client ready");
    Serial.println("Device MAC: " + api_getMacAddress());
  } else {
    Serial.println("❌ API Client failed - will retry in loop");
  }
  
  // Initialize core sensors
  Serial.println("\n--- Core Sensors Initialization ---");
  mpu_init();
  gps_init();
  
  // Initialize additional sensors (DHT11, MQ-7, Flame)
  Serial.println("\n--- Additional Sensors Initialization ---");
  sensors_init();
  
  // Try to initialize GSM module (for SMS and GPRS data)
  Serial.println("\n--- GSM Module Initialization ---");
  sim_init();
  
  // Check if SIM module is working and registered on network
  simAvailable = sim_isModuleWorking();
  if (simAvailable) {
    Serial.println("✅ SIM800L module is working and registered");
    
    // Initialize GPRS connection early (keep alive for telemetry)
    Serial.println("Initializing GPRS connection...");
    if (sim_initGPRS()) {
      Serial.println("✅ GPRS connection established and ready");
    } else {
      Serial.println("⚠ GPRS initialization failed - will retry on demand");
    }
  } else {
    Serial.println("⚠ SIM800L module not available or not registered");
  }
  
  // Initialize WiFi SMS Gateway as fallback
  Serial.println("\n--- SMS Gateway Initialization ---");
  sms_init();
  
  Serial.println("\n=================================");
  Serial.println("System Ready!");
  Serial.println("Telemetry upload every " + String(UPLOAD_INTERVAL_MS / 1000) + " seconds");
  Serial.println("=================================\n");
}

void loop() {
  // Update GPS data continuously
  gps_update();
  
  // Check for impact or tilt
  bool impact = mpu_detectImpact();
  bool tilt = mpu_detectTilt();
  
  // Read additional sensors
  bool fireDetected = sensors_isFireDetected();
  bool smokeDetected = sensors_isSmokeDetected();
  float temperature = sensors_getTemperature();
  bool extremeTempDetected = (temperature >= EXTREME_TEMP_THRESHOLD);
  
  // ===== ENVIRONMENTAL HAZARD SMS ALERTS =====
  // Send SMS when fire, smoke, or extreme temperature is detected
  
  // Fire detection SMS
  if (fireDetected && !fireSMSSent) {
    Serial.println("\n---------------------------------");
    Serial.println("FIRE DETECTED! Sending SMS...");
    Serial.println("---------------------------------");
    
    String mapsLink = gps_getGoogleMapsLink();
    String message = "ALERT: Fire detected! Location: " + mapsLink;
    
    bool smsSuccess = false;
    if (simAvailable) {
      smsSuccess = sim_sendSMS(PHONE_NUMBERS[0], message);
    }
    if (!smsSuccess) {
      smsSuccess = sms_sendSMS(PHONE_NUMBERS[0], message);
    }
    
    if (smsSuccess) {
      Serial.println("✓ Fire alert SMS sent");
      fireSMSSent = true;
    }
  } else if (!fireDetected && fireSMSSent) {
    fireSMSSent = false;  // Reset flag when fire is no longer detected
  }
  
  // Smoke detection SMS
  if (smokeDetected && !smokeSMSSent) {
    Serial.println("\n---------------------------------");
    Serial.println("SMOKE DETECTED! Sending SMS...");
    Serial.println("---------------------------------");
    
    String mapsLink = gps_getGoogleMapsLink();
    String message = "ALERT: Smoke detected! Location: " + mapsLink;
    
    bool smsSuccess = false;
    if (simAvailable) {
      smsSuccess = sim_sendSMS(PHONE_NUMBERS[0], message);
    }
    if (!smsSuccess) {
      smsSuccess = sms_sendSMS(PHONE_NUMBERS[0], message);
    }
    
    if (smsSuccess) {
      Serial.println("✓ Smoke alert SMS sent");
      smokeSMSSent = true;
    }
  } else if (!smokeDetected && smokeSMSSent) {
    smokeSMSSent = false;  // Reset flag when smoke is no longer detected
  }
  
  // Extreme temperature SMS
  if (extremeTempDetected && !extremeTempSMSSent) {
    Serial.println("\n---------------------------------");
    Serial.print("EXTREME TEMPERATURE DETECTED: ");
    Serial.print(temperature);
    Serial.println("°C! Sending SMS...");
    Serial.println("---------------------------------");
    
    String mapsLink = gps_getGoogleMapsLink();
    String message = "ALERT: Extreme temperature (" + String(temperature, 1) + "C) detected! Location: " + mapsLink;
    
    bool smsSuccess = false;
    if (simAvailable) {
      smsSuccess = sim_sendSMS(PHONE_NUMBERS[0], message);
    }
    if (!smsSuccess) {
      smsSuccess = sms_sendSMS(PHONE_NUMBERS[0], message);
    }
    
    if (smsSuccess) {
      Serial.println("✓ Extreme temperature alert SMS sent");
      extremeTempSMSSent = true;
    }
  } else if (!extremeTempDetected && extremeTempSMSSent) {
    extremeTempSMSSent = false;  // Reset flag when temperature returns to normal
  }
  
  // If accident detected and not already processed
  if ((impact || tilt) && !accidentDetected) {
    accidentDetected = true;
    Serial.println("\n---------------------------------");
    Serial.println("ACCIDENT DETECTED!");
    Serial.println("---------------------------------");
    
    // IMMEDIATELY send telemetry to Firebase to record the impact
    Serial.println("Sending impact telemetry to Firebase...");
    
    double lat = gps_getLatitude();
    double lng = gps_getLongitude();
    float speed = gps_getSpeed();
    
    // Get MPU6050 raw data while impact is still present
    int16_t ax, ay, az, gx, gy, gz;
    mpu_getRawData(&ax, &ay, &az, &gx, &gy, &gz);
    
    float humidity = sensors_getHumidity();
    int mq7Level = sensors_getMQ7Smooth();
    String gpsTimestamp = gps_getTimestamp();
    
    // Send telemetry immediately via available method with impact flag set to true
    bool impactTelemetrySent = false;
    if (GPRS_PRIMARY_MODE && simAvailable) {
      impactTelemetrySent = api_sendTelemetryViaGPRS(
        lat, lng, speed,
        fireDetected, smokeDetected,
        true,  // hasImpact = true
        ax, ay, az, gx, gy, gz,
        temperature, humidity, mq7Level, gpsTimestamp
      );
    }
    
    if (!impactTelemetrySent && api_isConnected()) {
      impactTelemetrySent = api_sendTelemetry(
        lat, lng, speed,
        fireDetected, smokeDetected,
        true,  // hasImpact = true
        ax, ay, az, gx, gy, gz,
        temperature, humidity, mq7Level, gpsTimestamp
      );
    }
    
    if (impactTelemetrySent) {
      Serial.println("✓ Impact telemetry sent to Firebase");
    } else {
      Serial.println("✗ Failed to send impact telemetry");
    }
    
    // Wait a moment for vehicle to come to a stop
    delay(100);
  }
  
  // If accident was detected, check if vehicle has stopped
  if (accidentDetected && gps_isStopped()) {
    Serial.println("\n*** VEHICLE STOPPED - SENDING LOCATION ***");
    
    // Get and print Google Maps link
    String mapsLink = gps_getGoogleMapsLink();
    double lat = gps_getLatitude();
    double lng = gps_getLongitude();
    
    Serial.println("Google Maps Link: " + mapsLink);
    Serial.print("Latitude: ");
    Serial.println(lat, 6);
    Serial.print("Longitude: ");
    Serial.println(lng, 6);
    
    // Route SMS based on SIM module availability
    bool smsSuccess = false;
    
    if (simAvailable) {
      // SIM module is working - send via GSM network
      Serial.println("Sending SMS via GSM network...");
      smsSuccess = sim_sendAccidentAlert(mapsLink);
      
      if (smsSuccess) {
        Serial.println("✓ SMS sent via GSM successfully!");
      } else {
        Serial.println("✗ GSM SMS failed, trying WiFi SMS Gateway...");
      }
    } else {
      Serial.println("SIM module not available, using WiFi SMS Gateway...");
    }
    
    // Fallback to WiFi SMS Gateway if SIM not working or GSM SMS failed
    if (!smsSuccess) {
      smsSuccess = sms_sendAccidentAlert(mapsLink);
      if (smsSuccess) {
        Serial.println("✓ SMS sent via WiFi Gateway");
      } else {
        Serial.println("✗ All SMS methods failed!");
      }
    }
    
    // Reset accident flag after processing
    accidentDetected = false;
  }
  
  // ===== TELEMETRY UPLOAD TO BACKEND =====
  unsigned long currentMillis = millis();
  if (currentMillis - lastTelemetryUpload >= UPLOAD_INTERVAL_MS) {
    lastTelemetryUpload = currentMillis;
    
    // Gather all sensor data
    double lat = gps_getLatitude();
    double lng = gps_getLongitude();
    float speed = gps_getSpeed();
    
    // Get MPU6050 raw data
    int16_t ax, ay, az, gx, gy, gz;
    mpu_getRawData(&ax, &ay, &az, &gx, &gy, &gz);
    
    // Debug: Print accelerometer values
    Serial.print("Accel: X=");
    Serial.print(ax);
    Serial.print(" Y=");
    Serial.print(ay);
    Serial.print(" Z=");
    Serial.print(az);
    Serial.print(" | Fire=");
    Serial.print(fireDetected);
    Serial.print(" Smoke=");
    Serial.println(smokeDetected);
    
    // Temperature already read earlier in loop
    float humidity = sensors_getHumidity();
    int mq7Level = sensors_getMQ7Smooth();
    String gpsTimestamp = gps_getTimestamp();
    
    bool telemetrySent = false;
    
    // GPRS Primary Mode - Try GPRS first, WiFi as fallback
    if (GPRS_PRIMARY_MODE && simAvailable) {
      Serial.println("Attempting telemetry via GPRS (primary)...");
      telemetrySent = api_sendTelemetryViaGPRS(
        lat, lng, speed,
        fireDetected, smokeDetected,
        false,  // hasImpact = false
        ax, ay, az, gx, gy, gz,
        temperature, humidity, mq7Level, gpsTimestamp
      );
      
      // Fallback to WiFi if GPRS failed
      if (!telemetrySent) {
        Serial.println("GPRS failed, attempting telemetry via WiFi...");
        if (!api_isConnected()) {
          Serial.println("WiFi disconnected, reconnecting...");
          api_init();
        }
        
        if (api_isConnected()) {
          telemetrySent = api_sendTelemetry(
            lat, lng, speed,
            fireDetected, smokeDetected,
            false,  // hasImpact = false
            ax, ay, az, gx, gy, gz,
            temperature, humidity, mq7Level, gpsTimestamp
          );
        }
      }
    } else {
      // WiFi Primary Mode (legacy) - Try WiFi first, GPRS as fallback
      if (!api_isConnected()) {
        Serial.println("WiFi disconnected, reconnecting...");
        api_init();
      }
      
      if (api_isConnected()) {
        Serial.println("Attempting telemetry via WiFi (primary)...");
        telemetrySent = api_sendTelemetry(
          lat, lng, speed,
          fireDetected, smokeDetected,
          false,  // hasImpact = false
          ax, ay, az, gx, gy, gz,
          temperature, humidity, mq7Level, gpsTimestamp
        );
      }
      
      // Fallback to GPRS if WiFi failed
      if (!telemetrySent && simAvailable) {
        Serial.println("WiFi failed, attempting telemetry via GPRS...");
        telemetrySent = api_sendTelemetryViaGPRS(
          lat, lng, speed,
          fireDetected, smokeDetected,
          false,  // hasImpact = false
          ax, ay, az, gx, gy, gz,
          temperature, humidity, mq7Level, gpsTimestamp
        );
      }
    }
    
    if (!telemetrySent) {
      Serial.println("✗ Telemetry upload failed on all methods");
    }
  }
  
  delay(100);  // Small delay for stability
}