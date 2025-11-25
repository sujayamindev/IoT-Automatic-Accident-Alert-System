#include "SIM.h"
#include "Config.h"

#define SIM_RX 16
#define SIM_TX 17

HardwareSerial sim800l(2);

// GPRS state management
bool gprsInitialized = false;
bool gprsConnected = false;

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

int sim_getSignalStrength() {
  sim800l.println("AT+CSQ");
  delay(500);
  
  String response = "";
  unsigned long start = millis();
  while (millis() - start < 1000) {
    while (sim800l.available()) {
      response += (char)sim800l.read();
    }
  }
  
  // Parse response: +CSQ: <rssi>,<ber>
  // rssi: 0-31 (signal strength), 99 = not known or not detectable
  int csqIndex = response.indexOf("+CSQ:");
  if (csqIndex != -1) {
    int commaIndex = response.indexOf(",", csqIndex);
    if (commaIndex != -1) {
      String rssiStr = response.substring(csqIndex + 6, commaIndex);
      rssiStr.trim();
      int rssi = rssiStr.toInt();
      return rssi;
    }
  }
  
  return 0;  // Return 0 if unable to read
}

bool sim_isModuleWorking() {
  // Send AT command
  sim800l.println("AT");
  delay(500);
  
  String response = "";
  unsigned long start = millis();
  while (millis() - start < 1000) {
    while (sim800l.available()) {
      response += (char)sim800l.read();
    }
  }
  
  // Check if module responds with OK
  if (response.indexOf("OK") != -1) {
    // Check network registration
    sim800l.println("AT+CREG?");
    delay(500);
    
    response = "";
    start = millis();
    while (millis() - start < 1000) {
      while (sim800l.available()) {
        response += (char)sim800l.read();
      }
    }
    
    // Check if registered (,1 = home network, ,5 = roaming)
    if (response.indexOf(",1") != -1 || response.indexOf(",5") != -1) {
      return true;
    }
  }
  
  return false;
}

bool sim_initGPRS() {
  // If already connected, return success
  if (gprsConnected) {
    Serial.println("GPRS already connected");
    return true;
  }
  
  Serial.println("\n=== Initializing GPRS connection ===");
  
  // Close any existing connection
  Serial.println("Step 1: Closing existing connections (AT+CIPSHUT)...");
  sim800l.println("AT+CIPSHUT");
  delay(2000);
  String response = "";
  while (sim800l.available()) {
    char c = sim800l.read();
    response += c;
    Serial.write(c);
  }
  Serial.println("CIPSHUT Response: " + response);
  
  // Set connection mode to single IP connection
  Serial.println("Step 2: Setting single connection mode (AT+CIPMUX=0)...");
  sim800l.println("AT+CIPMUX=0");
  delay(1000);
  response = "";
  while (sim800l.available()) {
    char c = sim800l.read();
    response += c;
    Serial.write(c);
  }
  Serial.println("CIPMUX Response: " + response);
  
  // Configure APN
  Serial.println("Step 3: Configuring APN (AT+CSTT)...");
  Serial.print("APN: ");
  Serial.println(GPRS_APN);
  sim800l.print("AT+CSTT=\"");
  sim800l.print(GPRS_APN);
  sim800l.print("\",\"");
  sim800l.print(GPRS_USERNAME);
  sim800l.print("\",\"");
  sim800l.print(GPRS_PASSWORD);
  sim800l.println("\"");
  delay(2000);
  
  response = "";
  while (sim800l.available()) {
    char c = sim800l.read();
    response += c;
    Serial.print(c);
  }
  Serial.println("CSTT Response: " + response);
  
  if (response.indexOf("ERROR") != -1) {
    Serial.println("✗ APN configuration failed!");
    return false;
  }
  Serial.println("✓ APN configured");
  
  // Bring up wireless connection
  Serial.println("Step 4: Bringing up wireless connection (AT+CIICR)...");
  Serial.println("This may take 5-15 seconds...");
  sim800l.println("AT+CIICR");
  
  response = "";
  unsigned long start = millis();
  bool gotResponse = false;
  while (millis() - start < 15000) {  // Increased timeout to 15 seconds
    while (sim800l.available()) {
      char c = sim800l.read();
      response += c;
      Serial.print(c);
    }
    if (response.indexOf("OK") != -1 || response.indexOf("ERROR") != -1) {
      gotResponse = true;
      break;
    }
    delay(100);
  }
  
  if (!gotResponse) {
    Serial.println("✗ Timeout waiting for GPRS connection!");
    return false;
  }
  
  if (response.indexOf("ERROR") != -1) {
    Serial.println("✗ GPRS connection failed!");
    Serial.println("Response: " + response);
    return false;
  }
  Serial.println("✓ GPRS connection established");
  
  // Get local IP address
  Serial.println("Step 5: Getting IP address (AT+CIFSR)...");
  sim800l.println("AT+CIFSR");
  delay(2000);
  
  response = "";
  while (sim800l.available()) {
    char c = sim800l.read();
    response += c;
    Serial.print(c);
  }
  Serial.println("CIFSR Response: " + response);
  
  // Check if we got a valid IP (not ERROR)
  if (response.indexOf("ERROR") != -1 || response.length() < 7) {
    Serial.println("✗ Failed to get IP address!");
    return false;
  }
  
  Serial.println("✓✓✓ GPRS connected successfully! ✓✓✓");
  Serial.println("==================================\n");
  gprsInitialized = true;
  gprsConnected = true;
  return true;
}

bool sim_sendHTTPPost(String url, String jsonPayload) {
  // Ensure GPRS is initialized
  if (!gprsConnected && !sim_initGPRS()) {
    Serial.println("✗ Cannot send HTTP: GPRS not connected");
    return false;
  }
  
  Serial.println("\n=== Sending HTTP POST via GPRS ===");
  Serial.println("URL: " + url);
  Serial.println("Payload size: " + String(jsonPayload.length()) + " bytes");
  
  // Setup bearer profile for HTTP (important!)
  Serial.println("Setting up bearer profile...");
  sim800l.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(500);
  while (sim800l.available()) Serial.write(sim800l.read());
  
  sim800l.print("AT+SAPBR=3,1,\"APN\",\"");
  sim800l.print(GPRS_APN);
  sim800l.println("\"");
  delay(500);
  while (sim800l.available()) Serial.write(sim800l.read());
  
  sim800l.println("AT+SAPBR=1,1");
  delay(2000);
  while (sim800l.available()) Serial.write(sim800l.read());
  
  // Initialize HTTP service
  Serial.println("Initializing HTTP service...");
  sim800l.println("AT+HTTPINIT");
  delay(1000);
  String response = "";
  while (sim800l.available()) {
    response += (char)sim800l.read();
  }
  Serial.print("HTTP INIT Response: ");
  Serial.println(response);
  
  if (response.indexOf("ERROR") != -1) {
    Serial.println("✗ HTTP INIT failed, trying to terminate first...");
    sim800l.println("AT+HTTPTERM");
    delay(1000);
    while (sim800l.available()) Serial.write(sim800l.read());
    
    sim800l.println("AT+HTTPINIT");
    delay(1000);
    response = "";
    while (sim800l.available()) {
      response += (char)sim800l.read();
    }
    if (response.indexOf("ERROR") != -1) {
      Serial.println("✗ HTTP INIT still failed");
      return false;
    }
  }
  Serial.println("✓ HTTP service initialized");
  
  // Set HTTP parameters - CID (bearer profile identifier)
  sim800l.println("AT+HTTPPARA=\"CID\",1");
  delay(500);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  // Set URL
  sim800l.print("AT+HTTPPARA=\"URL\",\"");
  sim800l.print(url);
  sim800l.println("\"");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  // Set content type to JSON
  sim800l.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  delay(500);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  // Set HTTP data
  int payloadSize = jsonPayload.length();
  sim800l.print("AT+HTTPDATA=");
  sim800l.print(payloadSize);
  sim800l.println(",10000");  // 10 second timeout for data input
  delay(1000);
  
  // Wait for DOWNLOAD prompt
  response = "";
  unsigned long start = millis();
  bool downloadReady = false;
  while (millis() - start < 5000) {
    while (sim800l.available()) {
      char c = sim800l.read();
      response += c;
      Serial.print(c);
      if (response.indexOf("DOWNLOAD") != -1) {
        downloadReady = true;
        break;
      }
    }
    if (downloadReady) break;
  }
  
  if (!downloadReady) {
    Serial.println("No DOWNLOAD prompt - aborting");
    sim800l.println("AT+HTTPTERM");
    delay(500);
    return false;
  }
  
  // Send JSON payload
  Serial.println("Sending payload...");
  sim800l.print(jsonPayload);
  delay(2000);
  
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  // Execute HTTP POST (method 1 = POST)
  Serial.println("Executing POST request...");
  sim800l.println("AT+HTTPACTION=1");
  
  // Wait for response (can take 10-30 seconds)
  response = "";
  start = millis();
  bool actionComplete = false;
  while (millis() - start < GPRS_TIMEOUT_MS) {
    while (sim800l.available()) {
      char c = sim800l.read();
      response += c;
      Serial.print(c);
      if (response.indexOf("+HTTPACTION:") != -1) {
        actionComplete = true;
        break;
      }
    }
    if (actionComplete) break;
    delay(100);
  }
  
  // Parse response: +HTTPACTION: 1,<status_code>,<data_length>
  bool success = false;
  if (response.indexOf("+HTTPACTION:") != -1) {
    int statusStart = response.indexOf(",") + 1;
    int statusEnd = response.indexOf(",", statusStart);
    if (statusStart > 0 && statusEnd > statusStart) {
      String statusStr = response.substring(statusStart, statusEnd);
      int statusCode = statusStr.toInt();
      
      Serial.print("HTTP Status Code: ");
      Serial.println(statusCode);
      
      if (statusCode == 200 || statusCode == 201) {
        Serial.println("✓ HTTP POST successful via GPRS");
        success = true;
      } else if (statusCode == 403) {
        Serial.println("✗ Device not bound (403)");
      } else {
        Serial.print("✗ HTTP error: ");
        Serial.println(statusCode);
      }
    }
  } else {
    Serial.println("✗ No HTTP response received");
  }
  
  // Read response data (optional)
  if (success) {
    sim800l.println("AT+HTTPREAD");
    delay(1000);
    Serial.println("Response data:");
    while (sim800l.available()) {
      Serial.write(sim800l.read());
    }
  }
  
  // Terminate HTTP service (but keep GPRS connected)
  sim800l.println("AT+HTTPTERM");
  delay(500);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  return success;
}

void sim_terminateGPRS() {
  if (!gprsConnected) {
    return;
  }
  
  Serial.println("Terminating GPRS connection...");
  
  // Close HTTP if still open
  sim800l.println("AT+HTTPTERM");
  delay(500);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  // Shut down GPRS connection
  sim800l.println("AT+CIPSHUT");
  delay(2000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  
  gprsConnected = false;
  gprsInitialized = false;
  Serial.println("GPRS terminated");
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
