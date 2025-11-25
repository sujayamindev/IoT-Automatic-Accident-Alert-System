#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Initialize all sensors
void sensors_init();

// Read DHT11 sensor
float sensors_getTemperature();
float sensors_getHumidity();

// Read MQ-7 CO sensor
int sensors_getCOLevel();
bool sensors_isCODetected();

// Read Flame sensor
int sensors_getFlameLevel();
bool sensors_isFireDetected();

#endif
