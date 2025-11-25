#ifndef SMS_H
#define SMS_H

#include <Arduino.h>

// Initialize WiFi and SMS gateway
void sms_init();

// Send SMS with accident alert and location via WiFi gateway
// Returns true if at least one SMS was sent successfully, false otherwise
bool sms_sendAccidentAlert(String mapsLink);

// Check if WiFi is connected
bool sms_isConnected();

#endif
