// Configuration for IoT Accident Alert System

#ifndef CONFIG_H
#define CONFIG_H

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

// Impact Detection Thresholds
#define IMPACT_THRESHOLD 22.0    // m/s² (≈ 2.2g impact)
#define TILT_THRESHOLD 55.0      // degrees (vehicle fall angle)

// Upload Intervals
#define UPLOAD_INTERVAL 1000           // Upload sensor data every 1 second
#define ACCIDENT_DEBOUNCE 5000         // 5 seconds debounce between accidents
#define ACCIDENT_UPLOAD_DELAY 5000     // Wait 5 seconds after detection before uploading

#endif // CONFIG_H
