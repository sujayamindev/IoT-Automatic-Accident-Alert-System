#ifndef GPS_H
#define GPS_H

#include <TinyGPS++.h>
#include <HardwareSerial.h>

// GPS UART pins
#define GPS_RX 19    // ESP32 receives from GPS TX
#define GPS_TX 18    // ESP32 sends to GPS RX
#define GPS_BAUD 9600

// Speed threshold for stopped detection (km/h)
const float STOPPED_SPEED_THRESHOLD = 5.0;

// Function declarations
void gps_init();
void gps_update();
bool gps_isStopped();
String gps_getGoogleMapsLink();
float gps_getLatitude();
float gps_getLongitude();
float gps_getSpeed();
bool gps_isLocationValid();

#endif
