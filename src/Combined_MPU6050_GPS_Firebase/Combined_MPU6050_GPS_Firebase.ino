// Combined test for MPU6050 and GPS Module with Firebase Integration
// Uploads sensor data to Firebase Realtime Database

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// WiFi Credentials 
#define WIFI_SSID "1234"
#define WIFI_PASSWORD "mindev1239"

// Firebase Credentials 
#define API_KEY "AIzaSyCfF-QCBXmlP1sBFvm3j-LfYha-wUqfUGc"
#define DATABASE_URL "https://iot-project-fa54e-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "test@test.com"
#define USER_PASSWORD "test1234"

// GPS Serial Configuration
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

// Create sensor objects
Adafruit_MPU6050 mpu;
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

// Firebase Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Time Tracking
unsigned long sendDataPrevMillis = 0;
const unsigned long uploadInterval = 1000; // Upload every 1 second

// Impact Detection Thresholds
float impactThreshold = 22.0;    // m/s² (≈ 2.2g impact)
float tiltThreshold = 55.0;      // degrees (vehicle fall angle)

// Accident Detection Variables
unsigned long lastAccidentTime = 0;
const unsigned long accidentDebounce = 5000; // 5 seconds debounce
unsigned long accidentDetectionTime = 0; // Time when accident was first detected
const unsigned long accidentUploadDelay = 5000; // Wait 5 seconds after detection before uploading
bool accidentInProgress = false; // Track if accident is being processed
bool lastImpactStatus = false;
bool lastFallStatus = false;
bool lastRollOrTiltStatus = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== MPU6050 & GPS Module with Firebase ===");
  Serial.println();
  
  // Initialize WiFi
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  // Initialize MPU6050
  Serial.println("Initializing MPU6050...");
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  
  // Configure MPU6050
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;
  }
  
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG:
      Serial.println("+- 2000 deg/s");
      break;
  }
  
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
      Serial.println("260 Hz");
      break;
    case MPU6050_BAND_184_HZ:
      Serial.println("184 Hz");
      break;
    case MPU6050_BAND_94_HZ:
      Serial.println("94 Hz");
      break;
    case MPU6050_BAND_44_HZ:
      Serial.println("44 Hz");
      break;
    case MPU6050_BAND_21_HZ:
      Serial.println("21 Hz");
      break;
    case MPU6050_BAND_10_HZ:
      Serial.println("10 Hz");
      break;
    case MPU6050_BAND_5_HZ:
      Serial.println("5 Hz");
      break;
  }
  
  Serial.println();
  
  // Initialize GPS
  Serial.println("Initializing GPS Module...");
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("GPS Serial started at 9600 baud rate");
  Serial.println();
  
  // Firebase Configuration
  Serial.println("Configuring Firebase...");
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  
  // Reconnect network
  Firebase.reconnectNetwork(true);
  fbdo.setResponseSize(2048);
  
  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;
  
  Serial.println("Firebase configured!");
  Serial.println();
  Serial.println("=== Starting Readings & Upload ===");
  Serial.println();
  delay(100);
}

void loop() {
  // Read MPU6050 data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Display MPU6050 readings
  Serial.println("--- MPU6050 Data ---");
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");
  
  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");
  
  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");
  Serial.println();
  
  // Calculate Impact Metrics
  float totalAcc = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );
  
  // Calculate tilt angles
  float roll = atan2(a.acceleration.y, a.acceleration.z) * 57.3;
  float pitch = atan2(-a.acceleration.x,
                sqrt(a.acceleration.y * a.acceleration.y +
                     a.acceleration.z * a.acceleration.z)) * 57.3;
  
  Serial.println("--- Impact Detection ---");
  Serial.print("Total Acceleration: ");
  Serial.print(totalAcc);
  Serial.println(" m/s^2");
  Serial.print("Roll: ");
  Serial.print(roll);
  Serial.print(" | Pitch: ");
  Serial.println(pitch);
  
  // Detect impact and fall
  bool impactDetected = (totalAcc > impactThreshold);
  bool fallDetected = (abs(roll) > tiltThreshold || abs(pitch) > tiltThreshold);
  bool accidentDetected = (impactDetected || fallDetected);
  
  if (impactDetected) {
    Serial.println("⚠️ IMPACT DETECTED!");
  }
  if (fallDetected) {
    Serial.println("⚠️ FALL / TILT DETECTED!");
  }
  if (accidentDetected) {
    Serial.println("🚨 ACCIDENT DETECTED 🚨");
  }
  Serial.println();
  
  // Read GPS data
  Serial.println("--- GPS Data ---");
  unsigned long start = millis();
  bool gpsDataReceived = false;
  
  while (millis() - start < 1000) {
    while (gpsSerial.available() > 0) {
      if (gps.encode(gpsSerial.read())) {
        gpsDataReceived = true;
      }
    }
  }
  
  bool gpsValid = false;
  if (gps.location.isValid()) {
    gpsValid = true;
    Serial.print("LAT: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("LONG: ");
    Serial.println(gps.location.lng(), 6);
    Serial.print("SPEED (km/h): ");
    Serial.println(gps.speed.kmph());
    Serial.print("ALTITUDE (m): ");
    Serial.println(gps.altitude.meters());
    Serial.print("HDOP: ");
    Serial.println(gps.hdop.value() / 100.0);
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());
    
    if (gps.date.isValid() && gps.time.isValid()) {
      Serial.print("Time in UTC: ");
      Serial.print(gps.date.year());
      Serial.print("/");
      Serial.print(gps.date.month());
      Serial.print("/");
      Serial.print(gps.date.day());
      Serial.print(", ");
      Serial.print(gps.time.hour());
      Serial.print(":");
      Serial.print(gps.time.minute());
      Serial.print(":");
      Serial.println(gps.time.second());
    }
  } else {
    Serial.println("GPS: Waiting for signal...");
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());
    if (gps.charsProcessed() < 10) {
      Serial.println("Warning: No GPS data received. Check wiring.");
    }
  }
  
  Serial.println();
  
  // Upload to Firebase
  if (Firebase.ready() && (millis() - sendDataPrevMillis > uploadInterval || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    
    Serial.println("--- Uploading to Firebase ---");
    
    // Upload MPU6050 Acceleration Data
    if (Firebase.RTDB.setFloat(&fbdo, "/sensor/mpu6050/acceleration/x", a.acceleration.x)) {
      Serial.println("✓ Accel X uploaded");
    } else {
      Serial.println("✗ Accel X failed: " + fbdo.errorReason());
    }
    
    if (Firebase.RTDB.setFloat(&fbdo, "/sensor/mpu6050/acceleration/y", a.acceleration.y)) {
      Serial.println("✓ Accel Y uploaded");
    } else {
      Serial.println("✗ Accel Y failed: " + fbdo.errorReason());
    }
    
    if (Firebase.RTDB.setFloat(&fbdo, "/sensor/mpu6050/acceleration/z", a.acceleration.z)) {
      Serial.println("✓ Accel Z uploaded");
    } else {
      Serial.println("✗ Accel Z failed: " + fbdo.errorReason());
    }
    
    // Upload MPU6050 Gyro Data
    if (Firebase.RTDB.setFloat(&fbdo, "/sensor/mpu6050/gyro/x", g.gyro.x)) {
      Serial.println("✓ Gyro X uploaded");
    } else {
      Serial.println("✗ Gyro X failed: " + fbdo.errorReason());
    }
    
    if (Firebase.RTDB.setFloat(&fbdo, "/sensor/mpu6050/gyro/y", g.gyro.y)) {
      Serial.println("✓ Gyro Y uploaded");
    } else {
      Serial.println("✗ Gyro Y failed: " + fbdo.errorReason());
    }
    
    if (Firebase.RTDB.setFloat(&fbdo, "/sensor/mpu6050/gyro/z", g.gyro.z)) {
      Serial.println("✓ Gyro Z uploaded");
    } else {
      Serial.println("✗ Gyro Z failed: " + fbdo.errorReason());
    }
    
    // Upload Temperature
    if (Firebase.RTDB.setFloat(&fbdo, "/sensor/mpu6050/temperature", temp.temperature)) {
      Serial.println("✓ Temperature uploaded");
    } else {
      Serial.println("✗ Temperature failed: " + fbdo.errorReason());
    }
    
    // Upload GPS Data (only if valid)
    if (gpsValid) {
      if (Firebase.RTDB.setDouble(&fbdo, "/sensor/gps/latitude", gps.location.lat())) {
        Serial.println("✓ Latitude uploaded");
      } else {
        Serial.println("✗ Latitude failed: " + fbdo.errorReason());
      }
      
      if (Firebase.RTDB.setDouble(&fbdo, "/sensor/gps/longitude", gps.location.lng())) {
        Serial.println("✓ Longitude uploaded");
      } else {
        Serial.println("✗ Longitude failed: " + fbdo.errorReason());
      }
      
      if (Firebase.RTDB.setFloat(&fbdo, "/sensor/gps/speed", gps.speed.kmph())) {
        Serial.println("✓ Speed uploaded");
      } else {
        Serial.println("✗ Speed failed: " + fbdo.errorReason());
      }
      
      if (Firebase.RTDB.setFloat(&fbdo, "/sensor/gps/altitude", gps.altitude.meters())) {
        Serial.println("✓ Altitude uploaded");
      } else {
        Serial.println("✗ Altitude failed: " + fbdo.errorReason());
      }
      
      if (Firebase.RTDB.setInt(&fbdo, "/sensor/gps/satellites", gps.satellites.value())) {
        Serial.println("✓ Satellites uploaded");
      } else {
        Serial.println("✗ Satellites failed: " + fbdo.errorReason());
      }
      
      if (Firebase.RTDB.setFloat(&fbdo, "/sensor/gps/hdop", gps.hdop.value() / 100.0)) {
        Serial.println("✓ HDOP uploaded");
      } else {
        Serial.println("✗ HDOP failed: " + fbdo.errorReason());
      }
      
      // Upload GPS timestamp if available
      if (gps.date.isValid() && gps.time.isValid()) {
        String timestamp = String(gps.date.year()) + "/" + 
                          String(gps.date.month()) + "/" + 
                          String(gps.date.day()) + " " +
                          String(gps.time.hour()) + ":" + 
                          String(gps.time.minute()) + ":" + 
                          String(gps.time.second());
        
        if (Firebase.RTDB.setString(&fbdo, "/sensor/gps/timestamp", timestamp)) {
          Serial.println("✓ GPS Timestamp uploaded");
        } else {
          Serial.println("✗ GPS Timestamp failed: " + fbdo.errorReason());
        }
      }
    } else {
      // Upload GPS status as invalid
      if (Firebase.RTDB.setString(&fbdo, "/sensor/gps/status", "No Signal")) {
        Serial.println("✓ GPS Status uploaded (No Signal)");
      } else {
        Serial.println("✗ GPS Status failed: " + fbdo.errorReason());
      }
    }
    
    Serial.println("--- Upload Complete ---");
  }
  
  // Accident Detection Logic with Delayed Upload
  if (accidentDetected && !accidentInProgress && (millis() - lastAccidentTime > accidentDebounce))
  {
    // Accident just detected, start timer
    accidentInProgress = true;
    accidentDetectionTime = millis();
    lastImpactStatus = impactDetected;
    lastFallStatus = fallDetected;
    lastRollOrTiltStatus = (abs(roll) > tiltThreshold || abs(pitch) > tiltThreshold);
    
    Serial.println();
    Serial.println("🚨 ACCIDENT DETECTED - Waiting to upload... 🚨");
  }
  
  // Upload Accident Data to Firebase after delay
  if (Firebase.ready() && accidentInProgress && (millis() - accidentDetectionTime >= accidentUploadDelay))
  {
    lastAccidentTime = millis();
    accidentInProgress = false;
    
    Serial.println();
    Serial.println("🚨 UPLOADING ACCIDENT STATUS TO FIREBASE 🚨");
    
    // Upload accident status
    if (Firebase.RTDB.setBool(&fbdo, "/accidents/latest/impactDetected", lastImpactStatus)) {
      Serial.println("✓ Impact status uploaded");
    } else {
      Serial.println("✗ Impact status failed: " + fbdo.errorReason());
    }
    
    if (Firebase.RTDB.setBool(&fbdo, "/accidents/latest/fallDetected", lastFallStatus)) {
      Serial.println("✓ Fall status uploaded");
    } else {
      Serial.println("✗ Fall status failed: " + fbdo.errorReason());
    }
    
    if (Firebase.RTDB.setBool(&fbdo, "/accidents/latest/rollOrTiltDetected", lastRollOrTiltStatus)) {
      Serial.println("✓ Roll/Tilt status uploaded");
    } else {
      Serial.println("✗ Roll/Tilt status failed: " + fbdo.errorReason());
    }
    
    Serial.println("🚨 ACCIDENT UPLOAD COMPLETE 🚨");
  }
  
  Serial.println();
  Serial.println("================================");
  Serial.println();
}
