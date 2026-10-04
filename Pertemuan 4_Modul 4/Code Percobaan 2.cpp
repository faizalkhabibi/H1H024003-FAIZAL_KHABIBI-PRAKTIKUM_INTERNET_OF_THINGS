#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "Sumbangan";
const char* password = "kvaratskhelia";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* topicData = "makan";
const char* topicPerintah = "makan";

const int ledPin = 26;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long waktuTerakhirPublish = 0;
const long intervalPublish = 5000; // publish data setiap 5 detik (non-blocking)

void callback(char* topic, byte* payload, unsigned int length) {
  String pesan;
  for (unsigned int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }

  JsonDocument doc;
  if (deserializeJson(doc, pesan)) {
    return; // abaikan jika parsing gagal
  }

  const char* perintah = doc["perintah"];
  digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);
  
  Serial.print("Perintah diterima -> Aktuator: ");
  Serial.println(perintah);
}

void hubungkanWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi berhasil terhubung!");
}

void hubungkanMQTT() {
  while (!client.connected()) {
    String clientId = "ESP8266Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe(topicPerintah);
      Serial.println("Terhubung dan subscribe topic perintah");
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  hubungkanWiFi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    hubungkanMQTT();
  }
  client.loop(); // memproses pesan masuk secara terus-menerus

  // Publish data dummy berkala tanpa sensor DHT
  if (millis() - waktuTerakhirPublish > intervalPublish) {
    waktuTerakhirPublish = millis();

    // Generasi nilai suhu dummy (misal 25.0°C - 35.0°C)
    float suhuDummy = random(250, 350) / 10.0;

    JsonDocument doc;
    doc["suhu"] = suhuDummy;
    
    char buffer[128];
    serializeJson(doc, buffer);
    
    client.publish(topicData, buffer);
    Serial.print("Data terkirim: ");
    Serial.println(buffer);
  }
}