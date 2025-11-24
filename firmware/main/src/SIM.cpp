#include "SIM.h"
#include "Config.h"

#define SIM_RX 16
#define SIM_TX 17

HardwareSerial sim800l(2);

void sim_init() {
  sim800l.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);
  
  Serial.println("SIM800L Initializing...");
  delay(3000);
  
  // Check module
  sim800l.println("AT");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  // Check SIM
  sim800l.println("AT+CPIN?");
  delay(2000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  // Check signal
  sim800l.println("AT+CSQ");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  // Wait for network registration
  Serial.println("Waiting for network...");
  for (int i = 0; i < 30; i++) {
    sim800l.println("AT+CREG?");
    delay(1000);
    
    String response = "";
    while (sim800l.available()) {
      char c = sim800l.read();
      response += c;
      Serial.print(c);
    }
    
    if (response.indexOf(",1") != -1 || response.indexOf(",5") != -1) {
      Serial.println("Network registered!");
      break;
    }
    
    Serial.print("Waiting... ");
    Serial.println(i + 1);
    delay(2000);
  }
  
  // Show operator
  sim800l.println("AT+COPS?");
  delay(2000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  Serial.println("SIM800L Ready!");
}

bool sim_sendAccidentAlert(String mapsLink) {
  String message = "ACCIDENT ALERT!\nVehicle has stopped.\nLocation: " + mapsLink;
  bool success = false;
  
  // Loop through all phone numbers
  int numPhones = sizeof(PHONE_NUMBERS) / sizeof(PHONE_NUMBERS[0]);
  for (int i = 0; i < numPhones; i++) {
    Serial.print("Sending SMS to ");
    Serial.println(PHONE_NUMBERS[i]);
    
    // Set text mode
    sim800l.println("AT+CMGF=1");
    delay(1000);
    while (sim800l.available()) {
      Serial.write(sim800l.read());
    }
    
    // Start SMS command
    sim800l.print("AT+CMGS=\"");
    sim800l.print(PHONE_NUMBERS[i]);
    sim800l.println("\"");
    delay(1000);
    
    // Wait for > prompt
    String prompt = "";
    unsigned long start = millis();
    while (millis() - start < 5000) {
      while (sim800l.available()) {
        char c = sim800l.read();
        prompt += c;
        Serial.print(c);
      }
      if (prompt.indexOf(">") != -1) break;
    }
    
    if (prompt.indexOf(">") != -1) {
      Serial.println("Sending message...");
      
      // Send message
      sim800l.print(message);
      delay(100);
      sim800l.write(26);  // Ctrl+Z
      
      // Wait for confirmation
      start = millis();
      String response = "";
      while (millis() - start < 30000) {
        while (sim800l.available()) {
          char c = sim800l.read();
          response += c;
          Serial.print(c);
        }
        if (response.indexOf("+CMGS:") != -1) {
          Serial.println("SMS sent successfully!");
          success = true;
          break;
        }
        if (response.indexOf("ERROR") != -1) {
          Serial.println("SMS failed!");
          break;
        }
      }
    } else {
      Serial.println("No > prompt!");
    }
    
    delay(2000);  // Wait between messages
  }
  
  return success;
}
