// Combined test for MPU6050 and GPS Module
// Tests both sensors simultaneously
// Uploads data to ThingSpeak using bulk API

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <ThingSpeak.h>

// WiFi Configuration
const char* WIFI_SSID = "1234";      // Replace with your WiFi SSID
const char* WIFI_PASSWORD = "mindev1239";  // Replace with your WiFi password

// ThingSpeak Configuration
// Channel 1 - MPU6050 Data
unsigned long TS_CHANNEL_MPU = 3166038;  // Replace with your Channel ID
const char* TS_WRITE_API_KEY_MPU = "PFZ8V82CDJSSYEYI";  // Replace with your Write API Key

// Channel 2 - GPS Data
unsigned long TS_CHANNEL_GPS = 3171327;  // Replace with your Channel ID
const char* TS_WRITE_API_KEY_GPS = "D0PDM14YZ5OBRV6M";  // Replace with your Write API Key

// Buffer Configuration
#define UPLOAD_INTERVAL 15000  // Upload interval in milliseconds (15 seconds)
#define SAMPLE_INTERVAL 1000   // Sample interval in milliseconds (1 second)
#define MPU_BUFFER_SIZE (UPLOAD_INTERVAL / SAMPLE_INTERVAL)  // 15 readings per upload
#define GPS_BUFFER_SIZE (UPLOAD_INTERVAL / SAMPLE_INTERVAL)  // 15 readings per upload

// GPS Serial Configuration
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

// Data structures for buffering
struct MPU6050Data {
  unsigned long timestamp;
  float accel_x;
  float accel_y;
  float accel_z;
  float gyro_x;
  float gyro_y;
  float gyro_z;
  float temperature;
};

struct GPSData {
  unsigned long timestamp;
  double latitude;
  double longitude;
  float speed;
  float altitude;
  int satellites;
};

// Create sensor objects
Adafruit_MPU6050 mpu;
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
WiFiClient client;

// Buffers
MPU6050Data mpuBuffer[MPU_BUFFER_SIZE];
GPSData gpsBuffer[GPS_BUFFER_SIZE];
int mpuBufferIndex = 0;
int gpsBufferIndex = 0;
int mpuBufferCount = 0;
int gpsBufferCount = 0;

// Timing
unsigned long lastUploadTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== MPU6050 & GPS Module with ThingSpeak ===");
  Serial.println();
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  // Initialize ThingSpeak
  ThingSpeak.begin(client);
  Serial.println("ThingSpeak initialized");
  Serial.print("MPU Channel ID: ");
  Serial.println(TS_CHANNEL_MPU);
  Serial.print("GPS Channel ID: ");
  Serial.println(TS_CHANNEL_GPS);
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
  Serial.println("=== Starting Readings ===");
  Serial.println();
  delay(100);
}

// Function to store MPU6050 data in buffer
void storeMPUData(float ax, float ay, float az, float gx, float gy, float gz, float temp) {
  mpuBuffer[mpuBufferIndex].timestamp = millis();
  mpuBuffer[mpuBufferIndex].accel_x = ax;
  mpuBuffer[mpuBufferIndex].accel_y = ay;
  mpuBuffer[mpuBufferIndex].accel_z = az;
  mpuBuffer[mpuBufferIndex].gyro_x = gx;
  mpuBuffer[mpuBufferIndex].gyro_y = gy;
  mpuBuffer[mpuBufferIndex].gyro_z = gz;
  mpuBuffer[mpuBufferIndex].temperature = temp;
  
  mpuBufferIndex = (mpuBufferIndex + 1) % MPU_BUFFER_SIZE;
  if (mpuBufferCount < MPU_BUFFER_SIZE) {
    mpuBufferCount++;
  }
}

// Function to store GPS data in buffer
void storeGPSData(double lat, double lng, float spd, float alt, int sat) {
  gpsBuffer[gpsBufferIndex].timestamp = millis();
  gpsBuffer[gpsBufferIndex].latitude = lat;
  gpsBuffer[gpsBufferIndex].longitude = lng;
  gpsBuffer[gpsBufferIndex].speed = spd;
  gpsBuffer[gpsBufferIndex].altitude = alt;
  gpsBuffer[gpsBufferIndex].satellites = sat;
  
  gpsBufferIndex = (gpsBufferIndex + 1) % GPS_BUFFER_SIZE;
  if (gpsBufferCount < GPS_BUFFER_SIZE) {
    gpsBufferCount++;
  }
}

// Function to upload MPU6050 data to ThingSpeak using bulk API
void uploadMPUData() {
  if (mpuBufferCount == 0) {
    Serial.println("No MPU data to upload");
    return;
  }
  
  Serial.print("Uploading ");
  Serial.print(mpuBufferCount);
  Serial.println(" MPU6050 readings to ThingSpeak...");
  
  // Build updates string in CSV format
  String updates = "";
  int startIndex = (mpuBufferIndex - mpuBufferCount + MPU_BUFFER_SIZE) % MPU_BUFFER_SIZE;
  
  for (int i = 0; i < mpuBufferCount; i++) {
    int idx = (startIndex + i) % MPU_BUFFER_SIZE;
    
    // Format: timestamp,field1,field2,field3,field4,field5,field6,field7
    // Use relative time (seconds from last entry)
    if (i == 0) {
      updates += "0";  // First entry is 0 seconds
    } else {
      // Calculate delta_t in seconds
      float delta_t = (mpuBuffer[idx].timestamp - mpuBuffer[(startIndex + i - 1) % MPU_BUFFER_SIZE].timestamp) / 1000.0;
      updates += String(delta_t, 3);  // Include milliseconds precision
    }
    updates += ",";
    updates += String(mpuBuffer[idx].accel_x, 4) + ",";
    updates += String(mpuBuffer[idx].accel_y, 4) + ",";
    updates += String(mpuBuffer[idx].accel_z, 4) + ",";
    updates += String(mpuBuffer[idx].gyro_x, 4) + ",";
    updates += String(mpuBuffer[idx].gyro_y, 4) + ",";
    updates += String(mpuBuffer[idx].gyro_z, 4) + ",";
    updates += String(mpuBuffer[idx].temperature, 2);
    
    if (i < mpuBufferCount - 1) {
      updates += "|";  // Pipe separator between entries
    }
  }
  
  // Build the complete POST body
  String postData = "write_api_key=" + String(TS_WRITE_API_KEY_MPU);
  postData += "&time_format=relative";
  postData += "&updates=" + updates;
  
  // Send bulk update via HTTP POST
  if (client.connect("api.thingspeak.com", 80)) {
    client.println("POST /channels/" + String(TS_CHANNEL_MPU) + "/bulk_update.csv HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: ESP32");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(postData.length()));
    client.println();
    client.print(postData);
    
    // Wait for response
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("MPU Upload timeout!");
        client.stop();
        return;
      }
    }
    
    // Read response
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    Serial.println();
    
    client.stop();
    Serial.println("MPU data uploaded successfully!");
    
    // Clear buffer after successful upload
    mpuBufferCount = 0;
    mpuBufferIndex = 0;
  } else {
    Serial.println("MPU Upload failed - connection error");
  }
}

// Function to upload GPS data to ThingSpeak using bulk API
void uploadGPSData() {
  if (gpsBufferCount == 0) {
    Serial.println("No GPS data to upload");
    return;
  }
  
  Serial.print("Uploading ");
  Serial.print(gpsBufferCount);
  Serial.println(" GPS readings to ThingSpeak...");
  
  // Build updates string in CSV format
  String updates = "";
  int startIndex = (gpsBufferIndex - gpsBufferCount + GPS_BUFFER_SIZE) % GPS_BUFFER_SIZE;
  
  for (int i = 0; i < gpsBufferCount; i++) {
    int idx = (startIndex + i) % GPS_BUFFER_SIZE;
    
    // Format: timestamp,field1,field2,field3,field4,field5
    // Use relative time (seconds from last entry)
    if (i == 0) {
      updates += "0";  // First entry is 0 seconds
    } else {
      // Calculate delta_t in seconds
      float delta_t = (gpsBuffer[idx].timestamp - gpsBuffer[(startIndex + i - 1) % GPS_BUFFER_SIZE].timestamp) / 1000.0;
      updates += String(delta_t, 3);  // 3 decimal places for milliseconds
    }
    updates += ",";
    updates += String(gpsBuffer[idx].latitude, 6) + ",";
    updates += String(gpsBuffer[idx].longitude, 6) + ",";
    updates += String(gpsBuffer[idx].speed, 2) + ",";
    updates += String(gpsBuffer[idx].altitude, 2) + ",";
    updates += String(gpsBuffer[idx].satellites);
    
    if (i < gpsBufferCount - 1) {
      updates += "|";  // Pipe separator between entries
    }
  }
  
  // Build the complete POST body
  String postData = "write_api_key=" + String(TS_WRITE_API_KEY_GPS);
  postData += "&time_format=relative";
  postData += "&updates=" + updates;
  
  // Send bulk update via HTTP POST
  if (client.connect("api.thingspeak.com", 80)) {
    client.println("POST /channels/" + String(TS_CHANNEL_GPS) + "/bulk_update.csv HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: ESP32");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(postData.length()));
    client.println();
    client.print(postData);
    
    // Wait for response
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("GPS Upload timeout!");
        client.stop();
        return;
      }
    }
    
    // Read response
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    Serial.println();
    
    client.stop();
    Serial.println("GPS data uploaded successfully!");
    
    // Clear buffer after successful upload
    gpsBufferCount = 0;
    gpsBufferIndex = 0;
  } else {
    Serial.println("GPS Upload failed - connection error");
  }
}

void loop() {
  // Read MPU6050 data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Store MPU6050 data in buffer
  storeMPUData(a.acceleration.x, a.acceleration.y, a.acceleration.z,
               g.gyro.x, g.gyro.y, g.gyro.z, temp.temperature);
  
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
  Serial.print("Buffered: ");
  Serial.print(mpuBufferCount);
  Serial.println(" readings");
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
  
  if (gps.location.isValid()) {
    // Store GPS data in buffer
    storeGPSData(gps.location.lat(), gps.location.lng(), 
                 gps.speed.kmph(), gps.altitude.meters(), 
                 gps.satellites.value());
    
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
    Serial.print("Buffered: ");
    Serial.print(gpsBufferCount);
    Serial.println(" readings");
    
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
  
  // Check if it's time to upload data
  if (millis() - lastUploadTime >= UPLOAD_INTERVAL) {
    Serial.println();
    Serial.println("=== Upload Time ===");
    
    // Upload MPU6050 data
    uploadMPUData();
    delay(1000);  // Small delay between uploads
    
    // Upload GPS data
    uploadGPSData();
    
    // Update last upload time
    lastUploadTime = millis();
    
    Serial.println("=== Upload Complete ===");
    Serial.println();
  }
  
  Serial.println();
  Serial.println("================================");
  Serial.println();
  
  delay(SAMPLE_INTERVAL);  // Wait for next sample (1 second)
}
