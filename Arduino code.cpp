#include <SPI.h>
#include <LoRa.h>

// LoRa Module (SX1278)
#define LORA_SS 10
#define LORA_RST 9
#define LORA_DIO0 2

// Alert Buzzer (Optional)
#define BUZZER 4

// Accept only this worker ID
String validWorkerID = "Worker_1234";

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("⚠ LoRa Initialization Failed!");
    while (1);
  }

  Serial.println("✅ LoRa Receiver Initialized!");
  Serial.println("🔘 Press 'A' in Serial Monitor to send ALERT");
}

void loop() {
  // ✅ Read LoRa packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incomingData = "";
    while (LoRa.available()) {
      incomingData += (char)LoRa.read();
    }

    // Validate basic packet structure
    if (incomingData.indexOf(',') != -1 && incomingData.length() > 10) {
      String fields[7];
      int fieldIndex = 0;

      for (int i = 0; i < incomingData.length(); i++) {
        char c = incomingData[i];
        if (c == ',') {
          fieldIndex++;
        } else {
          fields[fieldIndex] += c;
        }
      }

      // ✅ Filter by Worker ID
      if (fields[0] == validWorkerID) {
        Serial.println("🧑‍🏭 Worker Data Received:");
        Serial.println("🆔 ID:                 " + fields[0]);
        Serial.println("🌡 Temperature (°C):   " + fields[1]);
        Serial.println("🛢 MQ4 (CH₄):          " + fields[2]);
        Serial.println("☠ MQ7 (CO):           " + fields[3]);
        Serial.println("💨 MQ135 (Air/H₂S):    " + fields[4]);
        Serial.print("💥 Fall Detected:      ");
        Serial.println(fields[5] == "1" ? "Yes" : "No");
        Serial.print("🚨 Auto Alert:         ");
        Serial.println(fields[6] == "1" ? "Yes" : "No");
        Serial.println("--------------------------------------------------");

        // Auto Alert → Activate buzzer
        if (fields[6] == "1") {
          digitalWrite(BUZZER, HIGH);
          delay(2000);
          digitalWrite(BUZZER, LOW);
        }
      }
    }
  }

  // ✅ Manual Alert from Serial Monitor (Press 'A')
  if (Serial.available()) {
    char key = Serial.read();
    if (key == 'A' || key == 'a') {
      Serial.println("🛑 Manual Alert Sent to Worker!");
      LoRa.beginPacket();
      LoRa.print("ALERT");
      LoRa.endPacket();

      digitalWrite(BUZZER, HIGH);
      delay(1000);
      digitalWrite(BUZZER, LOW);
    }
  }
}
