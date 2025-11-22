int flameAnalog = A0;
int value = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  value = analogRead(flameAnalog);

  Serial.print("Analog Values: ");
  Serial.println(value);

  delay(500);
}
