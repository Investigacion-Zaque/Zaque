#include "sd_logger.h"
#include "config.h"
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>

static SPIClass sdSPI(HSPI);

class SDCardLoggerImpl : public SDCardLogger {
public:
  void init() override {
    // Inicializar SPI para tarjeta SD
    sdSPI.begin(GPIO_SD_SCK, GPIO_SD_MISO, GPIO_SD_MOSI, GPIO_SD_CS);
    
    if (!SD.begin(GPIO_SD_CS, sdSPI)) {
#if ENABLE_DEBUG_SERIAL
      Serial.println("✗ SD initialization failed!");
      Serial.println("Revisa:");
      Serial.println("1) CS -> GPIO5");
      Serial.println("2) SCK -> GPIO23");
      Serial.println("3) MOSI -> GPIO18");
      Serial.println("4) MISO -> GPIO19");
      Serial.println("5) VCC correcto: 3.3V o 5V");
      Serial.println("6) GND común");
      Serial.println("7) La microSD está formateada en FAT32");
#endif
      return;
    }

    // Crear directorios si no existen
    ensureDirectoriesExist();

#if ENABLE_DEBUG_SERIAL
    Serial.println("✓ SD card initialized");
    uint8_t cardType = SD.cardType();
    Serial.print("Card type: ");
    if (cardType == CARD_MMC) Serial.println("MMC");
    else if (cardType == CARD_SD) Serial.println("SDSC");
    else if (cardType == CARD_SDHC) Serial.println("SDHC");
    else Serial.println("UNKNOWN");
    
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("Card size: %lluMB\n", cardSize);
#endif
  }

  bool logMeasurement(const Measurement& m) override {
    // Escribir en CSV
    if (!logToCSV(m)) {
#if ENABLE_DEBUG_SERIAL
      Serial.println("✗ Failed to write CSV");
#endif
      return false;
    }

    // Actualizar JSON de última medición
    if (!updateLatestJSON(m)) {
#if ENABLE_DEBUG_SERIAL
      Serial.println("✗ Failed to update latest.json");
#endif
      return false;
    }

#if ENABLE_DEBUG_SERIAL
    Serial.println("✓ Measurement logged to SD");
#endif
    return true;
  }

  bool readLatest(Measurement& m) override {
    File file = SD.open(SD_LATEST_FILE, FILE_READ);
    if (!file) return false;

    // Aquí se podría parsear JSON pero para MVP simplemente retornar false
    file.close();
    return false;
  }

  void shutdown() override {
    SD.end();
  }

private:
  bool ensureDirectoriesExist() {
    // SD.mkdir crea directorios si no existen
    if (!SD.exists("/logs")) {
      SD.mkdir("/logs");
    }
    return true;
  }

  bool logToCSV(const Measurement& m) {
    File file = SD.open(SD_MEASUREMENTS_FILE, FILE_APPEND);
    if (!file) {
      // Crear archivo si no existe, con encabezados
      file = SD.open(SD_MEASUREMENTS_FILE, FILE_WRITE);
      if (!file) {
#if ENABLE_DEBUG_SERIAL
        Serial.println("✗ Failed to create CSV file");
#endif
        return false;
      }
      
      // Escribir encabezado CSV
      file.println("timestamp,node_id,node_name,lat,lon,soil_temperature,soil_humidity,"
                   "electrical_conductivity,ph,nitrogen,phosphorus,potassium,battery_percent,recommendation");
    }

    // Escribir fila de datos
    char csv_line[512];
    snprintf(csv_line, sizeof(csv_line),
      "%lu,%s,%s,%.6f,%.6f,%.1f,%.1f,%.2f,%.2f,%d,%d,%d,%d,\"%s\"",
      m.timestamp_ms,
      m.node_id,
      m.node_name,
      m.latitude,
      m.longitude,
      m.soil_temperature,
      m.soil_humidity,
      m.electrical_conductivity,
      m.ph,
      m.nitrogen,
      m.phosphorus,
      m.potassium,
      m.battery_percent,
      m.recommendation
    );

    if (file.println(csv_line)) {
#if ENABLE_DEBUG_SERIAL
      Serial.printf("✓ CSV written: %.1f%% humidity\n", m.soil_humidity);
#endif
    } else {
#if ENABLE_DEBUG_SERIAL
      Serial.println("✗ Failed to write CSV line");
#endif
      file.close();
      return false;
    }

    file.close();
    return true;
  }

  bool updateLatestJSON(const Measurement& m) {
    // Leer archivo actual
    DynamicJsonDocument doc(2048);
    File file = SD.open(SD_LATEST_FILE, FILE_READ);
    
    if (file) {
      deserializeJson(doc, file);
      file.close();
    }

    // Actualizar timestamp
    char timestamp[32];
    snprintf(timestamp, sizeof(timestamp), "%lu", m.timestamp_ms);
    doc["updated_at"] = timestamp;

    // Buscar nodo en array
    JsonArray nodes = doc["nodes"].isNull() ? doc.createNestedArray("nodes") : doc["nodes"].as<JsonArray>();
     
    // Buscar si el nodo ya existe
    int node_index = -1;
    for (int i = 0; i < nodes.size(); i++) {
      JsonObject node = nodes[i];
      if (strcmp(node["node_id"], m.node_id) == 0) {
        node_index = i;
        break;
      }
    }

    // Si no existe, crear nuevo
    if (node_index == -1) {
      node_index = nodes.size();
      nodes.createNestedObject();
    }

    // Actualizar datos del nodo
    JsonObject node_obj = nodes[node_index];
    node_obj["node_id"] = m.node_id;
    node_obj["node_name"] = m.node_name;
    node_obj["role"] = m.role;
    node_obj["lat"] = m.latitude;
    node_obj["lon"] = m.longitude;
    node_obj["soil_temperature"] = m.soil_temperature;
    node_obj["soil_humidity"] = m.soil_humidity;
    node_obj["electrical_conductivity"] = m.electrical_conductivity;
    node_obj["ph"] = m.ph;
    node_obj["nitrogen"] = m.nitrogen;
    node_obj["phosphorus"] = m.phosphorus;
    node_obj["potassium"] = m.potassium;
    node_obj["battery_percent"] = m.battery_percent;
    node_obj["last_seen"] = timestamp;
    node_obj["recommendation"] = m.recommendation;

    // Escribir JSON actualizado
    file = SD.open(SD_LATEST_FILE, FILE_WRITE);
    if (!file) {
#if ENABLE_DEBUG_SERIAL
      Serial.println("✗ Failed to open latest.json for writing");
#endif
      return false;
    }

    if (serializeJson(doc, file) == 0) {
#if ENABLE_DEBUG_SERIAL
      Serial.println("✗ Failed to serialize JSON");
#endif
      file.close();
      return false;
    }

    file.close();
    return true;
  }
};

// Factory function
SDLogger* createSDLogger() {
  return new SDCardLoggerImpl();
}
