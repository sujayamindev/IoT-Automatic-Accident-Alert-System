// ESP32 + SIM800L SMS + Internet Check (Dialog SIM)

#define SIM_RX 16
#define SIM_TX 17

HardwareSerial sim800l(2);

String phoneNumbers[] = {"+94717016900"};
int numPhones = 1;

void sendCommand(String cmd, int wait = 2000) {
  sim800l.println(cmd);
  delay(wait);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
}

bool checkInternetConnection() {
  Serial.println("\n=== Checking Internet Connection ===");

  // Set APN (Dialog)
  sendCommand("AT+CSTT=\"dialogbb\"", 2000);

  // Bring up wireless connection
  sim800l.println("AT+CIICR");
  delay(5000);

  String resp = "";
  while (sim800l.available()) {
    char c = sim800l.read();
    resp += c;
    Serial.print(c);
  }

  if (resp.indexOf("ERROR") != -1) {
    Serial.println("CIICR failed! No internet.");
    return false;
  }

  // Get local IP address
  sim800l.println("AT+CIFSR");
  delay(2000);

  String ip = "";
  while (sim800l.available()) {
    char c = sim800l.read();
    ip += c;
    Serial.print(c);
  }

  // Validate IP
  if (ip.length() > 7 && ip.indexOf(".") != -1) {
    Serial.print("Internet OK! IP Address: ");
    Serial.println(ip);
    return true;
  } else {
    Serial.println("Failed to obtain IP. No Internet.");
    return false;
  }
}

void setup() {
  Serial.begin(115200);
  sim800l.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);

  Serial.println("SIM800L Initializing...");
  delay(3000);

  sendCommand("AT");           // Check module
  sendCommand("AT+CPIN?");     // Check SIM status
  sendCommand("AT+CSQ");       // Signal quality

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

  sendCommand("AT+COPS?");   // Show operator

  Serial.println("SIM800L Ready!");

  // New addition
  checkInternetConnection();
}

void loop() {
  String message = "ACCIDENT ALERT!\nVehicle has stopped.\nLocation: https://maps.google.com/maps?q=6.927079,79.861244";

  for (int i = 0; i < numPhones; i++) {
    Serial.print("Sending SMS to ");
    Serial.println(phoneNumbers[i]);

    // Set text mode
    sendCommand("AT+CMGF=1", 1000);

    // Start SMS command
    sim800l.print("AT+CMGS=\"");
    sim800l.print(phoneNumbers[i]);
    sim800l.println("\"");
    delay(1000);

    // Wait for ">"
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
      sim800l.print(message);
      delay(100);
      sim800l.write(26); // Ctrl+Z

      String response = "";
      start = millis();
      while (millis() - start < 30000) {
        while (sim800l.available()) {
          char c = sim800l.read();
          response += c;
          Serial.print(c);
        }
        if (response.indexOf("+CMGS:") != -1) {
          Serial.println("SMS sent successfully!");
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

    delay(2000);
  }

  Serial.println("Waiting 60 seconds...");
  delay(60000);
}