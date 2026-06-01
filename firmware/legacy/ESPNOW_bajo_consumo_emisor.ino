#include <ModbusMaster.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>   // Para fijar canal
#include "secrets.h"

// UART2 pins
#define RXD2 16
#define TXD2 17
#define RS485_BAUD 4800

// Power control
#define POWER_PIN 2
#define POWER_PIN2 4
// Deep Sleep
#define SLEEP_MINUTES 30
#define uS_TO_S_FACTOR 1000000ULL

// Peer MAC (receiver) — definida en secrets.h
uint8_t peerMac[6] = PEER_MAC;

const int analogInPin = A6;  // Analog input pin that the potentiometer is attached to
float sensorValue = 0;  // value read from the pot

// Modbus
ModbusMaster node;

// Data payload
typedef struct {
  uint8_t status;       // 0=OK, otro=código error
  float temperature;    // °C
  float humidity;       // %RH
  float conductivity;   // µS/cm
  float ph;             // pH
  uint16_t N;           // mg/kg
  uint16_t P;           // mg/kg
  uint16_t K;           // mg/kg
  uint32_t timestamp;   // unix time (si disponible, 0 si no)
  uint16_t bateria;   // nuevo campo
} sensor_payload_t;

sensor_payload_t payload;

// === Callback corregido para core 3.x ===
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("ESP-NOW envio a ");
  for (int i = 0; i < 6; i++) {
    if (i) Serial.printf("%s%02X", (i ? ":" : ""), info->des_addr[i]);
  }
  Serial.print(" -> ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FALLO");
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("=== Emisor ESP32 + TH-NPK ===");

  pinMode(POWER_PIN, OUTPUT);
  pinMode(POWER_PIN2, OUTPUT);
  digitalWrite(POWER_PIN, LOW);
  digitalWrite(POWER_PIN2, LOW);
  delay(50);

  // Power ON
  Serial.println("Encendiendo HW-519...");
  digitalWrite(POWER_PIN, HIGH);
  digitalWrite(POWER_PIN2, HIGH);
  delay(5000); // arranque

  // Modbus
  Serial2.begin(RS485_BAUD, SERIAL_8N1, RXD2, TXD2);
  node.begin(1, Serial2);

  // ESP-NOW en canal 9
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  Serial.println("Emisor fijado en canal 1");

  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP-NOW listo");
    esp_now_register_send_cb(OnDataSent);

    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, peerMac, 6);
    peerInfo.channel = 1;  // IMPORTANTE
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
  } else {
    Serial.println("Error inicializando ESP-NOW");
  }

  // Leer sensor
  uint8_t result = node.readHoldingRegisters(0x0000, 7);
  if (result == node.ku8MBSuccess) {
    payload.status = 0;
    payload.humidity = node.getResponseBuffer(0) / 10.0f;
    payload.temperature = node.getResponseBuffer(1) / 10.0f;
    payload.conductivity = node.getResponseBuffer(2);
    payload.ph = node.getResponseBuffer(3) / 10.0f;
    payload.N = node.getResponseBuffer(4);
    payload.P = node.getResponseBuffer(5);
    payload.K = node.getResponseBuffer(6);
    payload.timestamp = 0; // opcional
  } else {
    Serial.print("Error Modbus: "); Serial.println(result);
    payload.status = result;
    payload.humidity = payload.temperature = payload.conductivity = payload.ph = 0.0;
    payload.N = payload.P = payload.K = 0;
    payload.timestamp = 0;
  }

    // read the analog in value:
  sensorValue = analogRead(analogInPin)*1.695;
  // map it to the range of the analog out:
  payload.bateria = map(sensorValue, 2048, 4096, 0, 100);

  // Enviar
  esp_now_send(peerMac, (uint8_t*)&payload, sizeof(payload));
  delay(500);

  // Power OFF
  Serial.println("Apagando HW-519...");
  digitalWrite(POWER_PIN, LOW);
  digitalWrite(POWER_PIN2, LOW);

  // Deep sleep
  Serial.println("Durmiendo 30 min...");
  esp_sleep_enable_timer_wakeup(SLEEP_MINUTES * 60 * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {}
