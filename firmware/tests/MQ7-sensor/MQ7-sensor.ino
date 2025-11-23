#define MQ7_PIN 34   // MQ-7 analog output to ESP32 ADC

void setup() {
  Serial.begin(115200);
  pinMode(MQ7_PIN, INPUT);

  Serial.println("=== MQ-7 Sensor Test (ESP32) ===");
}

void loop() {
  int mqValue = analogRead(MQ7_PIN);   // Range: 0 – 4095 on ESP32

  Serial.print("MQ-7 Raw Value: ");
  Serial.println(mqValue);

  delay(500);
}
