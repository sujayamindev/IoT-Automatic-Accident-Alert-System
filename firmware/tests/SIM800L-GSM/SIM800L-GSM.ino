// ---------------------------
// ESP32 + SIM800L with Detailed Diagnostics
// ---------------------------
#define SIM_RX 16
#define SIM_TX 17

HardwareSerial sim800l(1);

String phoneNumber = "+94717016900";
String staticMessage = "Vehicle status: STOPPED.\nLocation: TBD";

// ---------------------------
// SEND AT AND PRINT FULL RESPONSE
// ---------------------------
String sendATCommand(String cmd, int timeout) {
  Serial.print("\n[CMD] >> ");
  Serial.println(cmd);
  
  sim800l.println(cmd);
  
  String response = "";
  unsigned long start = millis();
  
  while (millis() - start < timeout) {
    while (sim800l.available()) {
      char c = sim800l.read();
      response += c;
    }
  }
  
  Serial.print("[RSP] << ");
  Serial.println(response);
  
  return response;
}

// ---------------------------
// CHECK IF RESPONSE CONTAINS OK
// ---------------------------
bool isResponseOK(String response) {
  return response.indexOf("OK") != -1;
}

// ---------------------------
// FULL DIAGNOSTICS
// ---------------------------
void runDiagnostics() {
  Serial.println("\n========== SIM800L DIAGNOSTICS ==========\n");
  
  // 1. Basic AT test
  Serial.println("--- 1. Module Communication ---");
  String resp = sendATCommand("AT", 1000);
  if (isResponseOK(resp)) {
    Serial.println("[OK] Module responding");
  } else {
    Serial.println("[FAIL] Module not responding!");
  }
  
  // 2. Module info
  Serial.println("\n--- 2. Module Info ---");
  sendATCommand("ATI", 1000);  // Module identification
  sendATCommand("AT+GSV", 1000);  // Firmware version
  
  // 3. SIM card status
  Serial.println("\n--- 3. SIM Card Status ---");
  resp = sendATCommand("AT+CPIN?", 2000);
  if (resp.indexOf("READY") != -1) {
    Serial.println("[OK] SIM card ready");
  } else if (resp.indexOf("SIM PIN") != -1) {
    Serial.println("[FAIL] SIM needs PIN code!");
  } else if (resp.indexOf("SIM PUK") != -1) {
    Serial.println("[FAIL] SIM needs PUK code!");
  } else {
    Serial.println("[FAIL] SIM not detected or error");
  }
  
  // 4. Signal quality
  Serial.println("\n--- 4. Signal Quality ---");
  resp = sendATCommand("AT+CSQ", 2000);
  // CSQ returns: +CSQ: <rssi>,<ber>
  // rssi: 0-31 (higher=better), 99=unknown
  // 10-14 = OK, 15-19 = Good, 20-31 = Excellent
  int idx = resp.indexOf("+CSQ:");
  if (idx != -1) {
    int rssi = resp.substring(idx + 6, resp.indexOf(",", idx)).toInt();
    Serial.print("[INFO] RSSI: ");
    Serial.print(rssi);
    if (rssi == 99) Serial.println(" (No signal!)");
    else if (rssi < 10) Serial.println(" (Weak signal)");
    else if (rssi < 15) Serial.println(" (OK signal)");
    else if (rssi < 20) Serial.println(" (Good signal)");
    else Serial.println(" (Excellent signal)");
  }
  
  // 5. Network registration
  Serial.println("\n--- 5. Network Registration ---");
  resp = sendATCommand("AT+CREG?", 2000);
  // +CREG: <n>,<stat>
  // stat: 0=not registered, 1=registered home, 2=searching, 
  //       3=denied, 5=registered roaming
  if (resp.indexOf(",1") != -1) {
    Serial.println("[OK] Registered on home network");
  } else if (resp.indexOf(",5") != -1) {
    Serial.println("[OK] Registered (roaming)");
  } else if (resp.indexOf(",2") != -1) {
    Serial.println("[WAIT] Searching for network...");
  } else if (resp.indexOf(",3") != -1) {
    Serial.println("[FAIL] Registration denied!");
  } else {
    Serial.println("[FAIL] Not registered");
  }
  
  // 6. Operator info
  Serial.println("\n--- 6. Network Operator ---");
  sendATCommand("AT+COPS?", 3000);  // Current operator
  
  // 7. Get own number (if stored on SIM)
  Serial.println("\n--- 7. SIM Phone Number ---");
  sendATCommand("AT+CNUM", 2000);
  
  // 8. Check SMS service center
  Serial.println("\n--- 8. SMS Service Center ---");
  resp = sendATCommand("AT+CSCA?", 2000);
  if (resp.indexOf("+CSCA:") != -1) {
    Serial.println("[INFO] SMS center configured");
  } else {
    Serial.println("[WARN] SMS center may not be set!");
  }
  
  // 9. Check account balance (network specific)
  Serial.println("\n--- 9. SIM Balance Check ---");
  Serial.println("[INFO] Use USSD to check balance manually");
  
  // 10. List SMS storage
  Serial.println("\n--- 10. SMS Storage Status ---");
  sendATCommand("AT+CPMS?", 2000);
  
  Serial.println("\n========== DIAGNOSTICS COMPLETE ==========\n");
}

// ---------------------------
// SEND SMS WITH DETAILED LOGGING
// ---------------------------
bool sendSMS(String message) {
  Serial.println("\n>>>>>> SENDING SMS <<<<<<");
  Serial.print("To: ");
  Serial.println(phoneNumber);
  Serial.print("Message: ");
  Serial.println(message);
  
  // Set text mode
  String resp = sendATCommand("AT+CMGF=1", 1000);
  if (!isResponseOK(resp)) {
    Serial.println("[FAIL] Could not set text mode");
    return false;
  }
  
  // Start SMS
  Serial.print("\n[CMD] >> AT+CMGS=\"");
  Serial.print(phoneNumber);
  Serial.println("\"");
  
  sim800l.print("AT+CMGS=\"");
  sim800l.print(phoneNumber);
  sim800l.println("\"");
  
  delay(1000);
  
  // Wait for '>' prompt
  String prompt = "";
  unsigned long start = millis();
  while (millis() - start < 3000) {
    while (sim800l.available()) {
      char c = sim800l.read();
      prompt += c;
      Serial.print(c);
    }
    if (prompt.indexOf(">") != -1) {
      Serial.println("\n[OK] Got > prompt");
      break;
    }
  }
  
  if (prompt.indexOf(">") == -1) {
    Serial.println("\n[FAIL] No > prompt received!");
    Serial.println("[RSP] << " + prompt);
    return false;
  }
  
  // Send message content
  Serial.println("[CMD] >> (message content)");
  sim800l.print(message);
  delay(500);
  
  // Send Ctrl+Z
  Serial.println("[CMD] >> Ctrl+Z (0x1A)");
  sim800l.write(26);
  
  // Wait for response (can take 5-60 seconds!)
  Serial.println("[WAIT] Waiting for network response...");
  resp = "";
  start = millis();
  while (millis() - start < 30000) {  // 30 second timeout
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
      Serial.println("\n[FAIL] SMS failed with ERROR");
      return false;
    }
  }
  
  Serial.println("\n[FAIL] SMS timeout - no confirmation");
  return false;
}

// ---------------------------
// SETUP
// ---------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n================================");
  Serial.println("   ESP32 + SIM800L SMS Tester");
  Serial.println("================================");
  
  // Try different baud rates
  Serial.println("\nTrying 9600 baud...");
  sim800l.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(1000);
  sim800l.println("AT");
  delay(500);
  if (sim800l.available()) {
    Serial.println("[OK] Module found at 9600 baud");
  } else {
    Serial.println("[FAIL] No response at 9600, trying 115200...");
    sim800l.end();
    sim800l.begin(115200, SERIAL_8N1, SIM_RX, SIM_TX);
    delay(1000);
    sim800l.println("AT");
    delay(500);
    if (sim800l.available()) {
      Serial.println("[OK] Module found at 115200 baud");
    } else {
      Serial.println("[FAIL] No response at 115200 either!");
      Serial.println("\n!!! CHECK WIRING AND POWER SUPPLY !!!");
    }
  }
  
  // Clear buffer
  while(sim800l.available()) sim800l.read();
  
  Serial.println("\nWaiting for SIM800L to initialize...");
  delay(3000);
  
  // Run full diagnostics
  runDiagnostics();
  
  // Wait for network registration before sending SMS
  Serial.println("\n\n*** WAITING FOR NETWORK ***");
  int attempts = 0;
  bool registered = false;
  
  while (attempts < 30) {  // Try for 60 seconds
    String resp = sendATCommand("AT+CREG?", 2000);
    if (resp.indexOf(",1") != -1 || resp.indexOf(",5") != -1) {
      Serial.println("[OK] Network registered!");
      registered = true;
      break;
    }
    
    resp = sendATCommand("AT+CSQ", 1000);
    Serial.print("Attempt ");
    Serial.print(attempts + 1);
    Serial.println("/30 - Waiting for signal...");
    
    attempts++;
    delay(2000);
  }
  
  if (!registered) {
    Serial.println("[FAIL] Could not register to network after 60 seconds");
    Serial.println("Check antenna and SIM card!");
    return;
  }
  
  // Show signal strength
  sendATCommand("AT+CSQ", 1000);
  sendATCommand("AT+COPS?", 2000);
  
  // Try sending test SMS
  Serial.println("\n\n*** ATTEMPTING TEST SMS ***\n");
  if (sendSMS("Test from ESP32+SIM800L")) {
    Serial.println("\n*** SMS SENT SUCCESSFULLY ***");
  } else {
    Serial.println("\n*** SMS FAILED ***");
  }
}

// ---------------------------
// LOOP
// ---------------------------
void loop() {
  // Print any incoming data from SIM800L
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  // Type commands in Serial Monitor to send to SIM800L
  while (Serial.available()) {
    sim800l.write(Serial.read());
  }
}
