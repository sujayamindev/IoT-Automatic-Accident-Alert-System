#include "Sensors.h"
#include "Config.h"
#include <DHT.h>

// DHT11 sensor instance
DHT dht(DHT11_PIN, DHT11);

void sensors_init() {
  Serial.println("Initializing additional sensors...");
  
  // Initialize DHT11
  dht.begin();
  Serial.println("- DHT11 initialized on pin " + String(DHT11_PIN));
  
  // Initialize analog pins
  pinMode(MQ7_PIN, INPUT);
  Serial.println("- MQ-7 initialized on pin " + String(MQ7_PIN));
  
  pinMode(FLAME_PIN, INPUT);
  Serial.println("- Flame sensor initialized on pin " + String(FLAME_PIN));
  
  Serial.println("All sensors initialized!");
}

float sensors_getTemperature() {
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    Serial.println("Warning: Failed to read temperature from DHT11");
    return 0.0;
  }
  return temp;
}

float sensors_getHumidity() {
  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
    Serial.println("Warning: Failed to read humidity from DHT11");
    return 0.0;
  }
  return humidity;
}

int sensors_getCOLevel() {
  return analogRead(MQ7_PIN);
}

bool sensors_isCODetected() {
  int coLevel = sensors_getCOLevel();
  return coLevel > MQ7_THRESHOLD;
}

int sensors_getFlameLevel() {
  return analogRead(FLAME_PIN);
}

bool sensors_isFireDetected() {
  int flameLevel = sensors_getFlameLevel();
  return flameLevel < FLAME_THRESHOLD;  // Lower values indicate fire
}
