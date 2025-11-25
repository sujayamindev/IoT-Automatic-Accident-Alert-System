#include "Sensors.h"
#include "Config.h"
#include <DHT.h>

// DHT11 sensor instance
DHT dht(DHT11_PIN, DHT11);

void sensors_init() {
  Serial.println("Initializing additional sensors...");
  
  // Initialize DHT11
  pinMode(DHT11_PIN, INPUT_PULLUP);
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
    // Silent fail - DHT11 sensor may not be connected
    return 0.0;
  }
  return temp;
}

float sensors_getHumidity() {
  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
    // Silent fail - DHT11 sensor may not be connected
    return 0.0;
  }
  return humidity;
}

// Smooth MQ7 analog value by averaging 30 readings
int sensors_getMQ7Smooth() {
  long sum = 0;
  for (int i = 0; i < 30; i++) {
    sum += analogRead(MQ7_PIN);
    delay(2);
  }
  return sum / 30;
}

bool sensors_isSmokeDetected() {
  int mq7Value = sensors_getMQ7Smooth();
  return mq7Value > MQ7_SMOKE_THRESHOLD;
}

// Read flame sensor digital state
int sensors_getFlameState() {
  return digitalRead(FLAME_PIN);
}

bool sensors_isFireDetected() {
  int flameState = sensors_getFlameState();
  return flameState == LOW;  // LOW indicates fire detected
}
