/*
 * ZAQUE - Monitoreo Agrícola Local Distribuido
 * 
 * Firmware unificado para nodos MAIN y SENSOR
 * 
 * Arquitectura: Red WiFi local del celular → MAIN (web + API) ← SENSOR (mediciones)
 * Almacenamiento: Todos los datos en microSD local
 * 
 * Este firmware es compilado con DEVICE_ROLE definido en config.h
 * DEVICE_ROLE_MAIN:   Nodo principal con dashboard web
 * DEVICE_ROLE_SENSOR: Nodo sensor que envía datos al MAIN
 * 
 * Versión: 0.2.0
 */

#include "config.h"
#include "types.h"
#include "sensor_reader.h"
#include "gps_reader.h"
#include "sd_logger.h"
#include "wifi_manager.h"
#include "recommendations.h"

// Roles específicos
#if DEVICE_ROLE == DEVICE_ROLE_MAIN
  #include "web_server.h"
  #include "node_registry.h"
#endif

#if DEVICE_ROLE == DEVICE_ROLE_SENSOR
  #include "http_client.h"
#endif

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

SensorReader* sensor_reader = nullptr;
GPSReader* gps_reader = nullptr;
SDLogger* sd_logger = nullptr;
WiFiManager* wifi_manager = nullptr;
Recommendations* recommendations = nullptr;

#if DEVICE_ROLE == DEVICE_ROLE_MAIN
  Web_ServerInterface* web_server = nullptr;
  NodeRegistry* node_registry = nullptr;
#endif

#if DEVICE_ROLE == DEVICE_ROLE_SENSOR
  HTTP_ClientInterface* http_client = nullptr;
#endif

unsigned long last_measurement_time = 0;

// ============================================================================
// SETUP - Inicialización
// ============================================================================

void setup() {
  // Serial para debug
#if ENABLE_DEBUG_SERIAL
  Serial.begin(DEBUG_BAUD_RATE);
  delay(1000);
  
  Serial.println("\n\n=== ZAQUE Initialization ===");
  Serial.printf("Device Role: %s\n", 
    DEVICE_ROLE == DEVICE_ROLE_MAIN ? "MAIN" : "SENSOR");
  Serial.printf("Node ID: %s\n", NODE_ID);
  Serial.printf("Firmware: %s\n", FIRMWARE_VERSION);
#endif

  // Inicializar sensor de suelo
#if ENABLE_DEBUG_SERIAL
  Serial.println("→ Initializing soil sensor...");
#endif
  sensor_reader = createSensorReader();
  sensor_reader->init();

  // Inicializar GPS
#if ENABLE_DEBUG_SERIAL
  Serial.println("→ Initializing GPS...");
#endif
  gps_reader = createGPSReader();
  gps_reader->init();

  // Inicializar microSD
#if ENABLE_DEBUG_SERIAL
  Serial.println("→ Initializing SD card...");
#endif
  sd_logger = createSDLogger();
  sd_logger->init();

  // Conectar a WiFi
#if ENABLE_DEBUG_SERIAL
  Serial.println("→ Connecting to WiFi...");
#endif
  wifi_manager = createWiFiManager();
  wifi_manager->init(WIFI_SSID, WIFI_PASSWORD);

  // Inicializar recomendaciones
  recommendations = createRecommendationEngine();

  // Inicialización específica por rol
#if DEVICE_ROLE == DEVICE_ROLE_MAIN
  Serial.println("→ Starting web server...");
  web_server = createWebServer();
  web_server->init(MAIN_PORT);

  node_registry = createNodeRegistry();
  node_registry->init();
  
#endif

#if DEVICE_ROLE == DEVICE_ROLE_SENSOR
  Serial.println("→ Initializing HTTP client...");
  http_client = createHTTPClient();
  http_client->init(MAIN_HOST, MAIN_PORT);
#endif

#if ENABLE_DEBUG_SERIAL
  Serial.println("✓ Initialization complete");
#endif
}

// ============================================================================
// LOOP PRINCIPAL
// ============================================================================

void loop() {
  // Verificar si es hora de tomar una nueva medición
  if (millis() - last_measurement_time < (MEASUREMENT_INTERVAL_MINUTES * 60 * 1000)) {
    // Aún no es hora; hacer tareas del rol actual

#if DEVICE_ROLE == DEVICE_ROLE_MAIN
    // El MAIN está siempre atendiendo el servidor web
    web_server->handleRequests();
    delay(100); // Pequeño delay para no consumir CPU
#else
    // El SENSOR puede hacer otras tareas o dormir parcialmente
    delay(1000);
#endif
    return;
  }

  // Es hora de tomar medición
  last_measurement_time = millis();

#if ENABLE_DEBUG_SERIAL
  Serial.println("\n→ Taking measurement...");
#endif

  Measurement m;
  
  // Llenar metadatos
  strncpy(m.node_id, NODE_ID, sizeof(m.node_id) - 1);
  strncpy(m.node_name, NODE_NAME, sizeof(m.node_name) - 1);
  strncpy(m.role, DEVICE_ROLE == DEVICE_ROLE_MAIN ? "main" : "sensor", sizeof(m.role) - 1);
  strncpy(m.firmware_version, FIRMWARE_VERSION, sizeof(m.firmware_version) - 1);
  m.timestamp_ms = millis();

  // Leer sensor
  if (!sensor_reader->read(m)) {
#if ENABLE_DEBUG_SERIAL
    Serial.println("✗ Failed to read sensor");
#endif
    return;
  }

  // Leer GPS
  if (!gps_reader->read(m.latitude, m.longitude)) {
#if ENABLE_DEBUG_SERIAL
    Serial.println("! GPS not available, using last known location");
#endif
    m.gps_valid = false;
  } else {
    m.gps_valid = true;
  }

  // Generar recomendación
  recommendations->generateRecommendation(m, m.recommendation, sizeof(m.recommendation));

  // Guardar en SD local
#if ENABLE_DEBUG_SERIAL
  Serial.println("→ Saving to SD...");
#endif
  if (!sd_logger->logMeasurement(m)) {
#if ENABLE_DEBUG_SERIAL
    Serial.println("✗ Failed to save to SD");
#endif
  }

  // Acciones específicas por rol
#if DEVICE_ROLE == DEVICE_ROLE_MAIN
  // El MAIN guarda su propia medición en el registro de nodos
  node_registry->updateLocalNode(m);
  
#if ENABLE_DEBUG_SERIAL
  Serial.println("✓ Main measurement recorded");
#endif

#endif

#if DEVICE_ROLE == DEVICE_ROLE_SENSOR
  // El SENSOR envía la medición al MAIN
#if ENABLE_DEBUG_SERIAL
  Serial.println("→ Sending measurement to MAIN...");
#endif
  
  if (!http_client->sendWithRetry(m, SENSOR_SEND_MAX_RETRIES)) {
#if ENABLE_DEBUG_SERIAL
    Serial.println("! Failed to send to MAIN, saved for retry");
#endif
  }

  // Entrar en deep sleep
#if ENABLE_DEBUG_SERIAL
  Serial.printf("→ Going to deep sleep for %d minutes...\n", 
    MEASUREMENT_INTERVAL_MINUTES);
  Serial.flush();
#endif
  
  esp_sleep_enable_timer_wakeup(MEASUREMENT_INTERVAL_MINUTES * 60 * 1000000);
  esp_deep_sleep_start();
  
#endif
}

// ============================================================================
// NOTA: Implementar en archivos separados:
// - sensor_reader.cpp
// - gps_reader.cpp
// - sd_logger.cpp
// - wifi_manager.cpp
// - http_client.cpp (solo SENSOR)
// - web_server.cpp (solo MAIN)
// - node_registry.cpp (solo MAIN)
// - recommendations.cpp
// ============================================================================
