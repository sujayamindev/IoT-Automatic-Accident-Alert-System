#include "GPS.h"

TinyGPSPlus gps;
HardwareSerial gpsSerial(1);  // Use UART1

void gps_init() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("GPS UART1 initialized");
}

void gps_update() {
  // Continuously read available GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
}

bool gps_isStopped() {
  static unsigned long stoppedStartTime = 0;
  static bool wasStopped = false;
  const unsigned long STOPPED_DURATION_MS = 10000;  // 10 seconds
  
  if (!gps.speed.isValid()) {
    return false;  // Can't determine if no valid speed data
  }
  
  float currentSpeed = gps.speed.kmph();
  
  if (currentSpeed < STOPPED_SPEED_THRESHOLD) {
    // Vehicle is currently stopped
    if (!wasStopped) {
      // Just stopped, start the timer
      stoppedStartTime = millis();
      wasStopped = true;
    }
    
    // Check if stopped for required duration
    if (millis() - stoppedStartTime >= STOPPED_DURATION_MS) {
      Serial.println("Vehicle is STOPPED for " + String(STOPPED_DURATION_MS / 1000) + "+ seconds");
      return true;
    }
  } else {
    // Vehicle is moving, reset the timer
    wasStopped = false;
    stoppedStartTime = 0;
  }
  
  return false;
}

String gps_getGoogleMapsLink() {
  if (!gps.location.isValid()) {
    return "No GPS fix available";
  }
  
  double lat = gps.location.lat();
  double lng = gps.location.lng();
  
  String mapsLink = "https://www.google.com/maps?q=" + String(lat, 6) + "," + String(lng, 6);
  
  return mapsLink;
}

float gps_getLatitude() {
  if (gps.location.isValid()) {
    return gps.location.lat();
  }
  return 0.0;
}

float gps_getLongitude() {
  if (gps.location.isValid()) {
    return gps.location.lng();
  }
  return 0.0;
}

float gps_getSpeed() {
  if (gps.speed.isValid()) {
    return gps.speed.kmph();
  }
  return 0.0;
}

bool gps_isLocationValid() {
  return gps.location.isValid();
}

String gps_getTimestamp() {
  if (!gps.date.isValid() || !gps.time.isValid()) {
    return "";
  }
  
  // Get UTC time from GPS
  int year = gps.date.year();
  int month = gps.date.month();
  int day = gps.date.day();
  int hour = gps.time.hour();
  int minute = gps.time.minute();
  int second = gps.time.second();
  
  // Convert to IST (+05:30)
  minute += 30;
  if (minute >= 60) {
    minute -= 60;
    hour += 1;
  }
  hour += 5;
  if (hour >= 24) {
    hour -= 24;
    day += 1;
    // Simple day overflow handling (doesn't account for month end)
  }
  
  // Format as ISO 8601: YYYY-MM-DDTHH:MM:SS+05:30
  char timestamp[35];
  sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02d+05:30", 
          year, month, day, hour, minute, second);
  return String(timestamp);
}

String gps_getFormattedTimestamp() {
  if (!gps.date.isValid() || !gps.time.isValid()) {
    return "No GPS time";
  }
  
  // Get UTC time from GPS
  int year = gps.date.year();
  int month = gps.date.month();
  int day = gps.date.day();
  int hour = gps.time.hour();
  int minute = gps.time.minute();
  int second = gps.time.second();
  
  // Convert to IST (+05:30)
  minute += 30;
  if (minute >= 60) {
    minute -= 60;
    hour += 1;
  }
  hour += 5;
  if (hour >= 24) {
    hour -= 24;
    day += 1;
  }
  
  char timestamp[35];
  sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d IST", 
          year, month, day, hour, minute, second);
  return String(timestamp);
}
