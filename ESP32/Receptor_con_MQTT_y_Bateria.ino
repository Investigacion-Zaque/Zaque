#include <WiFi.h>
#include <esp_now.h>
#include <PubSubClient.h>
#include <esp_wifi.h>
#include "secrets.h"

const int analogInPin = A6;  // Entrada analógica para batería
float sensorValue = 0;
int bateria = 0;

// --- Configuración WiFi ---
const char* WIFI_SSID_VAL     = WIFI_SSID;
const char* WIFI_PASSWORD_VAL = WIFI_PASSWORD;

// --- Configuración MQTT ---
const char* mqtt_server = MQTT_SERVER;
const int   mqtt_port   = MQTT_PORT;
const char* mqtt_user   = MQTT_USER;
const char* mqtt_pass   = MQTT_PASS;
const char* mqtt_topic = "clima/temperatura";
const char* mqtt_topic2 = "clima/receptor";

WiFiClient espClient;
PubSubClient client(espClient);

// --- Estructura ESPNOW ---
typedef struct {
  uint8_t status;
  float temperature;
  float humidity;
  float conductivity;
  float ph;
  uint16_t N;
  uint16_t P;
  uint16_t K;
  uint32_t timestamp;
  uint16_t bateria;
} sensor_payload_t;

sensor_payload_t incoming;

// --- Control reconexión MQTT ---
unsigned long lastReconnectAttempt = 0;
bool mqttWasConnected = false;

// --- Control de publicación de batería ---
unsigned long lastBatterySend = 0;
const unsigned long batteryInterval = 1800000; // cada 10 segundos

// --- Callback ESPNOW ---
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&incoming, incomingData, sizeof(incoming));

  Serial.printf("Datos recibidos de MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                info->src_addr[0], info->src_addr[1], info->src_addr[2],
                info->src_addr[3], info->src_addr[4], info->src_addr[5]);

  if (incoming.status == 0) {
    Serial.printf("  Temp: %.1f °C, Humedad: %.1f %%\n", incoming.temperature, incoming.humidity);

    if (client.connected()) {
      char payload[200];
      snprintf(payload, sizeof(payload),
               "{\"temp\": %.2f, \"hum\": %.2f, \"EC\": %.0f, \"pH\": %.2f, \"N\": %u, \"P\": %u, \"K\": %u , \"Bat\": %u}",
               incoming.temperature, incoming.humidity, incoming.conductivity,
               incoming.ph, incoming.N, incoming.P, incoming.K , incoming.bateria);

      client.publish(mqtt_topic, payload);
      Serial.println("Publicado en MQTT:");
      Serial.println(payload);
    } else {
      Serial.println("No conectado a MQTT, descartando publicación.");
    }
  } else {
    Serial.printf("  Error en lectura Modbus. Codigo: %u\n", incoming.status);
  }
}

// --- Función de reconexión MQTT ---
bool reconnect() {
  if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
    Serial.println("MQTT conectado!");
    mqttWasConnected = true;
  }
  return client.connected();
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID_VAL, WIFI_PASSWORD_VAL);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");

  // Fijar canal WiFi y ESPNOW
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicializando ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  // Configuración cliente MQTT
  client.setServer(mqtt_server, mqtt_port);
  lastReconnectAttempt = 0;

  Serial.println("Receptor listo con ESPNOW + MQTT");
}

void loop() {
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {  // cada 5 seg máx
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
    mqttWasConnected = false;
  } else {
    if (!mqttWasConnected) {
      Serial.println("MQTT conectado!");
      mqttWasConnected = true;
    }
    client.loop();
  }

  // --- Lectura analógica de batería ---
  sensorValue = analogRead(analogInPin) * 1.695;  // calibración
  bateria = map(sensorValue, 2048, 4096, 0, 100);

  // --- Envío periódico del valor de batería ---
  unsigned long now = millis();
  if (client.connected() && now - lastBatterySend > batteryInterval) {
    lastBatterySend = now;

    char payload[80];
    snprintf(payload, sizeof(payload), "{\"bateria\": %d}", bateria);

    client.publish(mqtt_topic2, payload);
    Serial.printf("Publicado valor de batería (%d%%) en %s\n", bateria, mqtt_topic2);
  }
}