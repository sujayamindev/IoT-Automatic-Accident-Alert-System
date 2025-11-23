#include <DHT.h>

#define DHTPIN 4       // Digital pin connected to DHT11 DATA
#define DHTTYPE DHT11  // DHT 11 sensor

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("=== DHT11 Sensor Test (ESP32, 3.3V) ===");
}

void loop() {
  float temperature = dht.readTemperature(); // Celsius
  float humidity = dht.readHumidity();

  // Check if any reads failed
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT11 sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C   |   Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  delay(2000); // DHT11 requires 1-2 seconds between readings
}
