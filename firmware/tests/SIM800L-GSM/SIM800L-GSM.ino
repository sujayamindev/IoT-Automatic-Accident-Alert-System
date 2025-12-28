// ESP32 + SIM800L SMS with Network Check

#define SIM_RX 16
#define SIM_TX 17

HardwareSerial sim800l(2);

String phoneNumber = "+94771234567";  // Replace with your phone number
String staticMessage = "Vehicle status: STOPPED.\nLocation: TBD";

// Send AT command and return response
String sendAT(String cmd, int timeout) {
  Serial.print("[CMD] ");
  Serial.println(cmd);
  
  // Clear buffer
  while (sim800l.available()) sim800l.read();
  
  sim800l.println(cmd);
  
  String response = "";
  unsigned long start = millis();
  
  while (millis() - start < timeout) {
    while (sim800l.available()) {
      char c = sim800l.read();
      response += c;
    }
    delay(10);
  }
  
  Serial.print("[RSP] ");
  Serial.println(response);
  return response;
}

// Wait for network registration
bool waitForNetwork(int maxSeconds) {
  Serial.println("\nWaiting for network...");
  
  for (int i = 0; i < maxSeconds; i++) {
    String resp = sendAT("AT+CREG?", 1000);
    
    if (resp.indexOf(",1") != -1) {
      Serial.println("[OK] Registered on home network!");
      return true;
    }
    if (resp.indexOf(",5") != -1) {
      Serial.println("[OK] Registered (roaming)!");
      return true;
    }
    
    // Check signal too
    sendAT("AT+CSQ", 1000);
    
    Serial.print("Waiting... ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.println(maxSeconds);
    
    delay(2000);
  }
  
  Serial.println("[FAIL] Could not register to network!");
  return false;
}

// Send SMS
bool sendSMS(String message) {
  Serial.println("\n--- Sending SMS ---");
  Serial.print("To: ");
  Serial.println(phoneNumber);
  
  // Set text mode
  String resp = sendAT("AT+CMGF=1", 1000);
  if (resp.indexOf("OK") == -1) {
    Serial.println("[FAIL] Could not set text mode");
    return false;
  }
  
  // Start SMS command
  sim800l.print("AT+CMGS=\"");
  sim800l.print(phoneNumber);
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
  
  if (prompt.indexOf(">") == -1) {
    Serial.println("\n[FAIL] No > prompt!");
    return false;
  }
  
  Serial.println("\n[OK] Got > prompt, sending message...");
  
  // Send message
  sim800l.print(message);
  delay(100);
  sim800l.write(26);  // Ctrl+Z
  
  // Wait for response
  Serial.println("Waiting for confirmation...");
  resp = "";
  start = millis();
  while (millis() - start < 30000) {
    while (sim800l.available()) {
      char c = sim800l.read();
      resp += c;
      Serial.print(c);
    }
    if (resp.indexOf("+CMGS:") != -1) {
      Serial.println("\n[OK] SMS sent successfully!");
      return true;
    }
    if (resp.indexOf("ERROR") != -1) {
      Serial.println("\n[FAIL] SMS error!");
      return false;
    }
  }
  
  Serial.println("\n[FAIL] SMS timeout!");
  return false;
}

void setup() {
  Serial.begin(115200);
  sim800l.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);
  
  Serial.println("\n================================");
  Serial.println("   ESP32 + SIM800L SMS");
  Serial.println("================================\n");
  
  delay(3000);
  
  // Check module
  sendAT("AT", 1000);
  
  // Check SIM
  sendAT("AT+CPIN?", 2000);
  
  // Check signal
  sendAT("AT+CSQ", 1000);
  
  // Wait for network (max 60 seconds)
  if (!waitForNetwork(30)) {
    Serial.println("\n!!! NO NETWORK - Check antenna and SIM !!!");
    Serial.println("SMS will not work without network.");
    return;
  }
  
  // Show operator
  sendAT("AT+COPS?", 2000);
  
  // Send test SMS
  Serial.println("\n*** Sending test SMS ***");
  sendSMS("SIM800L Test: Module is working!");
}

void loop() {
  // Only send if you want repeated SMS
  // sendSMS(staticMessage);
  // delay(60000);
  
  // For now, just pass through serial
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  while (Serial.available()) {
    sim800l.write(Serial.read());
  }
}