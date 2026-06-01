#include "web_server.h"
#include "config.h"
#include "dashboard_html.h"
#include "node_registry.h"
#include <ArduinoJson.h>
#include <SD.h>
#include <WiFi.h>

// Declaración forward del registry
extern NodeRegistry* getGlobalNodeRegistry();

// Variable global para el SD logger
static SDLogger* g_sd_logger = nullptr;

// Implementación de ESP32WebServer
void ESP32WebServer::init(int port) {
  server_port = port;

  server.on("/", HTTP_GET, [this]() { handleRoot(); });
  server.on("/dashboard", HTTP_GET, [this]() { handleRoot(); });
  server.on("/api/status", HTTP_GET, [this]() { handleStatusAPI(); });
  server.on("/api/nodes", HTTP_GET, [this]() { handleNodesAPI(); });
  server.on("/api/measurements/latest", HTTP_GET, [this]() { handleLatestAPI(); });
  server.on("/api/history", HTTP_GET, [this]() { handleHistoryAPI(); });
  server.on("/api/measurements", HTTP_POST, [this]() { handleMeasurementsAPI(); });
  server.on("/download/measurements.csv", HTTP_GET, [this]() { handleDownloadCSV(); });

  server.begin();

#if ENABLE_DEBUG_SERIAL
  Serial.printf("✓ Web server started on port %d\n", server_port);
  Serial.printf("  → http://zaque.local/\n");
#endif
}

void ESP32WebServer::handleRequests() {
  server.handleClient();
}

void ESP32WebServer::shutdown() {
  server.stop();
}

void ESP32WebServer::setSDLogger(SDLogger* logger) {
  g_sd_logger = logger;
  sd_logger = logger;
}

void ESP32WebServer::handleRoot() {
  server.send(200, "text/html", DASHBOARD_HTML);
}

void ESP32WebServer::handleStatusAPI() {
  DynamicJsonDocument doc(256);
  doc["status"] = "ok";
  doc["role"] = DEVICE_ROLE == DEVICE_ROLE_MAIN ? "main" : "sensor";
  doc["node_id"] = NODE_ID;
  doc["firmware"] = FIRMWARE_VERSION;
  doc["uptime_seconds"] = millis() / 1000;
  doc["connected_sensors"] = getGlobalNodeRegistry()->getNodeCount();
  doc["wifi_signal_strength"] = WiFi.RSSI();

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void ESP32WebServer::handleNodesAPI() {
  DynamicJsonDocument doc(1024);
  JsonArray nodes_array = doc.createNestedArray("nodes");

  NodeRegistry* registry = getGlobalNodeRegistry();
  for (int i = 0; i < registry->getNodeCount(); i++) {
    Measurement m;
    if (registry->getNodeByID(NODE_ID, m)) {
      JsonObject node = nodes_array.createNestedObject();
      node["node_id"] = m.node_id;
      node["node_name"] = m.node_name;
      node["role"] = m.role;
      node["battery_percent"] = m.battery_percent;
      node["is_active"] = true;
    }
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void ESP32WebServer::handleLatestAPI() {
  File file = SD.open(SD_LATEST_FILE, FILE_READ);
  if (!file) {
    server.send(404, "application/json", "{\"error\": \"latest.json not found\"}");
    return;
  }

  String content;
  while (file.available()) {
    content += (char)file.read();
  }
  file.close();

  server.send(200, "application/json", content);
}

void ESP32WebServer::handleHistoryAPI() {
  String node_id = server.arg("node_id");
  int limit = server.hasArg("limit") ? server.arg("limit").toInt() : 100;

  DynamicJsonDocument doc(4096);
  doc["node_id"] = node_id;
  JsonArray measurements = doc.createNestedArray("measurements");

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void ESP32WebServer::handleMeasurementsAPI() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\": \"No data\"}");
    return;
  }

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    server.send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
    return;
  }

  if (doc["api_key"] != API_KEY) {
    server.send(401, "application/json", "{\"error\": \"Invalid API key\"}");
    return;
  }

  Measurement m;
  strcpy(m.node_id, doc["node_id"] | "unknown");
  strcpy(m.node_name, doc["node_name"] | "Unknown");
  strcpy(m.role, doc["role"] | "sensor");
  m.latitude = doc["lat"] | 0.0;
  m.longitude = doc["lon"] | 0.0;
  m.soil_temperature = doc["soil_temperature"] | 0.0;
  m.soil_humidity = doc["soil_humidity"] | 0.0;
  m.electrical_conductivity = doc["electrical_conductivity"] | 0;
  m.ph = doc["ph"] | 0.0;
  m.nitrogen = doc["nitrogen"] | 0;
  m.phosphorus = doc["phosphorus"] | 0;
  m.potassium = doc["potassium"] | 0;
  m.battery_percent = doc["battery_percent"] | 0;
  m.timestamp_ms = millis();
  strcpy(m.firmware_version, FIRMWARE_VERSION);

  if (g_sd_logger) {
    g_sd_logger->logMeasurement(m);
  }

  NodeRegistry* registry = getGlobalNodeRegistry();
  registry->addRemoteNode(m);

  DynamicJsonDocument response(256);
  response["status"] = "ok";
  response["stored"] = true;
  response["recommendation"] = m.recommendation;
  response["server_time"] = millis();

  String resp;
  serializeJson(response, resp);
  server.send(200, "application/json", resp);
}

void ESP32WebServer::handleDownloadCSV() {
  File file = SD.open(SD_MEASUREMENTS_FILE, FILE_READ);
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  server.sendHeader("Content-Disposition", "attachment; filename=\"measurements.csv\"");
  server.setContentLength(file.size());
  server.send(200, "text/csv", "");

  uint8_t buf[512];
  size_t len;
  while ((len = file.read(buf, sizeof(buf))) > 0) {
    server.client().write(buf, len);
  }

  file.close();
}

// Factory function
Web_ServerInterface* createWebServer() {
  return new ESP32WebServer(MAIN_PORT);
}
