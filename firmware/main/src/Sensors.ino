#include <DHT.h>

// PIN DEFINITIONS FOR ESP32
#define FLAME_PIN 35      // Flame sensor digital output (input-only GPIO)
#define MQ7_PIN   34      // MQ7 analog output (ADC1 channel)
#define DHTPIN    4       // DHT11 data pin with internal pull-up
#define DHTTYPE   DHT11   // DHT11 sensor type

DHT dht(DHTPIN, DHTTYPE);

// FUNCTION: SMOOTH MQ7 ANALOG VALUE
int readMQ7smooth() {
  long sum = 0;
  for (int i = 0; i < 30; i++) {
    sum += analogRead(MQ7_PIN); // ESP32 ADC returns 0-4095
    delay(2);
  }
  return sum / 30;
}

// SETUP
void setup() {
  Serial.begin(115200);
  pinMode(FLAME_PIN, INPUT);        // Flame sensor input
  pinMode(DHTPIN, INPUT_PULLUP);    // DHT11 with internal pull-up
  dht.begin();
  delay(2000);
}

// MAIN LOOP
void loop() {

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int flameState = digitalRead(FLAME_PIN);
  int mq7Value = readMQ7smooth();

  bool fireDetected = false;
  bool smokeDetected = false;

  Serial.println("\n----- STATUS -----");

  // FLAME DETECTION
  if (flameState == LOW) {
    Serial.println("🔥 FLAME DETECTED!");
    fireDetected = true;
  } else {
    Serial.println("No Flame");
  }

  // SMOKE DETECTION
  if (mq7Value > 250) {  // Adjust threshold as needed
    Serial.println("💨 SMOKE DETECTED!");
    smokeDetected = true;
  } else {
    Serial.println("No Smoke");
  }

  // SHOW TEMPERATURE AND HUMIDITY
  if (!isnan(temperature)) {
    Serial.print("🌡 Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
  } else {
    Serial.println("❌ DHT READ ERROR");
  }
  
  if (!isnan(humidity)) {
    Serial.print("💧 Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  // SHOW COMBINED ALERT
  if (fireDetected && smokeDetected) {
    Serial.println("BOTH FIRE AND SMOKE DETECTED!");
  }

  Serial.println("-----------------------------");
  delay(2000);
}