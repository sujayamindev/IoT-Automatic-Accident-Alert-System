#ifndef SIM_H
#define SIM_H

#include <Arduino.h>

// Initialize SIM800L module
void sim_init();

// Check if SIM module is working and registered on network
bool sim_isModuleWorking();

// Initialize GPRS connection (keeps alive after first call)
// Returns true if successful, false if failed
bool sim_initGPRS();

// Send HTTP POST request via GPRS
// Returns true if successful (HTTP 200/201), false if failed
bool sim_sendHTTPPost(String url, String jsonPayload);

// Terminate GPRS connection (only call when shutting down)
void sim_terminateGPRS();

// Send SMS with accident alert and location
// Returns true if successful, false if failed
bool sim_sendAccidentAlert(String mapsLink);

#endif
