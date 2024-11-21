#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ThingSpeak.h>
#include <DHT.h>

// Define pins and thresholds
#define DHTPIN D7
#define DHTTYPE DHT11
#define MQ_PIN A0
#define LED_PIN D8 // Replace BUZZER_PIN with LED_PIN
#define TEMP_THRESHOLD 20.0
#define GAS_THRESHOLD 570

DHT dht(DHTPIN, DHTTYPE);

WiFiClient client;
const char* ssid = "sanyam";
const char* password = "123456789";
uint8_t broadcastAddress[] = {0x48, 0x3F, 0xDA, 0x74, 0x5F, 0x26};
String URL = "/update?api_key=5SK94NG8WP1K8GA7&field1=";

typedef struct struct_message {
    bool Beep; // Beep flag now controls LED
} struct_message;

struct_message messgo;

void setup() {
    Serial.begin(9600);
    dht.begin();
    pinMode(LED_PIN, OUTPUT); // Set LED pin as output

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(200);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");
    ThingSpeak.begin(client);

    // Initialize ESP-NOW
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
    // Read temperature and gas levels
    float temp = dht.readTemperature();
    float ppm = analogRead(MQ_PIN);

    // Determine if thresholds are crossed
    if (temp > TEMP_THRESHOLD || ppm > GAS_THRESHOLD) {
        digitalWrite(LED_PIN, HIGH); // Turn LED ON
        messgo.Beep = true;
    } else {
        digitalWrite(LED_PIN, LOW);  // Turn LED OFF
        messgo.Beep = false;
    }

    // Send status via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t*)&messgo, sizeof(messgo));

    // Log data to ThingSpeak
    sendData(temp, ppm);
    delay(15000);
}

void sendData(float temp, float ppm) {
    WiFiClient client1;
    String newUrl = URL + String(temp) + "&field2=" + String(ppm);
    if (client1.connect("api.thingspeak.com", 80)) {
        client1.print(String("GET ") + newUrl + " HTTP/1.1\r\n" +
                      "Host: api.thingspeak.com\r\n" +
                      "Connection: close\r\n\r\n");
        Serial.println("Data sent to ThingSpeak");
    }
    client1.stop();
}