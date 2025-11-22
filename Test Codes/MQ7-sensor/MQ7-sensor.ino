// MQ-7 CO Sensor with ESP32
// Connect MQ-7 Analog OUT → GPIO34 (or 35, 36, 39)

int mq7Pin = 34;   // Analog input pin for MQ-7

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("MQ-7 Sensor Test Starting...");
}

void loop() {
  int sensorValue = analogRead(mq7Pin);   // Read raw analog value (0–4095)
  
  Serial.print("MQ-7 Value: ");
  Serial.println(sensorValue);

  delay(500);
}
