#ifndef GPS_H
#define GPS_H

#include <TinyGPS++.h>
#include <HardwareSerial.h>

// GPS UART pins
#define GPS_RX 18    // ESP32 receives from GPS TX
#define GPS_TX 19   // ESP32 sends to GPS RX
#define GPS_BAUD 9600

// Speed threshold for stopped detection (km/h)
const float STOPPED_SPEED_THRESHOLD = 5.0;

// Function declarations
void gps_init();
void gps_update();
bool gps_isStopped();
String gps_getGoogleMapsLink();
String gps_getGoogleMapsLinkWithFallback();  // Get maps link with fallback to last valid location
float gps_getLatitude();
float gps_getLongitude();
float gps_getSpeed();
bool gps_isLocationValid();
bool gps_hasLastValidLocation();     // Check if we have any cached location
String gps_getTimestamp();           // Get GPS timestamp in ISO 8601 format with +0530 timezone
String gps_getFormattedTimestamp();  // Get human-readable GPS timestamp
String gps_getLastValidTimestamp();  // Get timestamp of last valid location

#endif
