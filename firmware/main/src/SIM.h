#ifndef SIM_H
#define SIM_H

#include <Arduino.h>

// Initialize SIM800L module
void sim_init();

// Send SMS with accident alert and location
// Returns true if successful, false if failed
bool sim_sendAccidentAlert(String mapsLink);

#endif
