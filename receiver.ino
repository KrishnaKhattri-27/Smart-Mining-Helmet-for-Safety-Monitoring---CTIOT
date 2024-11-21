#define LED_PIN D2  // Replace BUZZER_PIN with LED_PIN

#include <ESP8266WiFi.h>
#include <espnow.h>

// ESP-NOW Message Structure
typedef struct struct_message {
  bool Beep;  // LED signal
} struct_message;

struct_message messcome;  // Message received

uint8_t broadcastAddress[] = {0xC8, 0xC9, 0xA3, 0x56, 0x66, 0x76}; // Sender's MAC Address

// ESP-NOW Data Received Callback
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&messcome, incomingData, sizeof(messcome));
}

void setup() {
  // Setup Serial Monitor
  Serial.begin(9600);
  
  // Setup LED Pin
  pinMode(LED_PIN, OUTPUT);

  // Initialize Wi-Fi
  WiFi.disconnect();
  delay(1000);
  WiFi.mode(WIFI_STA);  // Set Wi-Fi to station mode (required for ESP-NOW)
  
  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Set ESP-NOW role and register callback
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Receiver initialized and waiting for data...");
}

void loop() {
  // Check the received Beep signal and control the LED
  if (messcome.Beep) {
    digitalWrite(LED_PIN, HIGH);  // Turn LED ON
    delay(1000);                 // LED ON duration
    digitalWrite(LED_PIN, LOW);  // Turn LED OFF
    delay(1000);                 // LED OFF duration
  } else {
    digitalWrite(LED_PIN, LOW);  // Ensure LED is OFF
  }

  // Debugging: Print Beep value
  Serial.println(messcome.Beep);
}