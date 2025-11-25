#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Initialize all sensors
void sensors_init();

// Read DHT11 sensor
float sensors_getTemperature();
float sensors_getHumidity();

// Read MQ-7 CO sensor with smoothing
int sensors_getMQ7Smooth();
bool sensors_isSmokeDetected();

// Read Flame sensor
int sensors_getFlameState();
bool sensors_isFireDetected();

#endif
