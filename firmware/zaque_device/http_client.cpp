#include "http_client.h"
#include "config.h"
#include <ArduinoJson.h>

void ESP32HTTPClient::init(const char* host, int port) {
  strcpy(main_host, host);
  main_port = port;
  snprintf(main_url, sizeof(main_url), "http://%s:%d/api/measurements", main_host, main_port);

#if ENABLE_DEBUG_SERIAL
  Serial.printf("✓ HTTP client initialized\n");
  Serial.printf("  → Target: %s\n", main_url);
#endif
}

bool ESP32HTTPClient::sendMeasurement(const Measurement& m) {
  return sendWithRetry(m, 1);
}

bool ESP32HTTPClient::sendWithRetry(const Measurement& m, int max_retries) {
  for (int attempt = 0; attempt < max_retries; attempt++) {
    // Construir payload JSON
    DynamicJsonDocument doc(512);
    doc["api_key"] = API_KEY;
    doc["node_id"] = m.node_id;
    doc["node_name"] = m.node_name;
    doc["role"] = m.role;
    doc["timestamp"] = m.timestamp_ms;
    doc["lat"] = m.latitude;
    doc["lon"] = m.longitude;
    doc["soil_temperature"] = m.soil_temperature;
    doc["soil_humidity"] = m.soil_humidity;
    doc["electrical_conductivity"] = m.electrical_conductivity;
    doc["ph"] = m.ph;
    doc["nitrogen"] = m.nitrogen;
    doc["phosphorus"] = m.phosphorus;
    doc["potassium"] = m.potassium;
    doc["battery_percent"] = m.battery_percent;
    doc["firmware_version"] = m.firmware_version;

    String payload;
    serializeJson(doc, payload);

#if ENABLE_DEBUG_SERIAL
    Serial.printf("→ Sending to %s (attempt %d)\n", main_url, attempt + 1);
#endif

    // Enviar HTTP POST
    http_client.setConnectTimeout(SENSOR_SEND_TIMEOUT_MS);
    http_client.setTimeout(SENSOR_SEND_TIMEOUT_MS);
    http_client.begin(main_url);
    http_client.addHeader("Content-Type", "application/json");

    int httpCode = http_client.POST(payload);

#if ENABLE_DEBUG_SERIAL
    Serial.printf("✓ HTTP response: %d\n", httpCode);
#endif

    if (httpCode == HTTP_CODE_OK) {
      String response = http_client.getString();
      
      DynamicJsonDocument resp_doc(256);
      if (deserializeJson(resp_doc, response) == DeserializationError::Ok) {
        if (resp_doc["status"] == "ok") {
          http_client.end();
          return true;
        }
      }
    }

    http_client.end();

    if (attempt < max_retries - 1) {
      unsigned long wait_ms = 1000 * (1 << attempt);
      delay(wait_ms);
    }
  }

  return false;
}

void ESP32HTTPClient::shutdown() {
  http_client.end();
}

bool ESP32HTTPClient::buildPayloadJSON(const Measurement& m, char* buffer, size_t size) {
  DynamicJsonDocument doc(512);
  doc["api_key"] = API_KEY;
  doc["node_id"] = m.node_id;
  doc["node_name"] = m.node_name;

  return serializeJson(doc, buffer, size) > 0;
}

bool ESP32HTTPClient::parseResponse(const char* response, char* recommendation, size_t rec_size) {
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, response) != DeserializationError::Ok) {
    return false;
  }

  if (doc.containsKey("recommendation")) {
    strncpy(recommendation, doc["recommendation"], rec_size - 1);
    return true;
  }

  return false;
}

// Factory function
HTTP_ClientInterface* createHTTPClient() {
  return new ESP32HTTPClient();
}
