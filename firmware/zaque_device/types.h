#pragma once

#include <Arduino.h>

// Estructura para almacenar una medición del suelo
struct Measurement {
  // Metadatos
  char node_id[32];
  char node_name[64];
  char role[16];              // "main" o "sensor"
  unsigned long timestamp_ms;
  
  // Ubicación GPS
  float latitude;
  float longitude;
  bool gps_valid;
  
  // Mediciones de suelo
  float soil_temperature;     // °C
  float soil_humidity;        // %
  float electrical_conductivity; // µS/cm
  float ph;
  
  // NPK (Nitrogen, Phosphorus, Potassium)
  int16_t nitrogen;           // mg/kg
  int16_t phosphorus;         // mg/kg
  int16_t potassium;          // mg/kg
  
  // Energía
  uint8_t battery_percent;    // 0-100
  
  // Versión del firmware
  char firmware_version[16];
  
  // Recomendación agrícola
  char recommendation[256];
};

// Interface para lectura de sensores
class SensorReader {
public:
  virtual void init() = 0;
  virtual bool read(Measurement& m) = 0;
  virtual void shutdown() = 0;
  virtual ~SensorReader() {}
};

// Interface para lectura de GPS
class GPSReader {
public:
  virtual void init() = 0;
  virtual bool read(float& latitude, float& longitude) = 0;
  virtual void shutdown() = 0;
  virtual ~GPSReader() {}
};

// Interface para logging en SD
class SDLogger {
public:
  virtual void init() = 0;
  virtual bool logMeasurement(const Measurement& m) = 0;
  virtual bool readLatest(Measurement& m) = 0;
  virtual void shutdown() = 0;
  virtual ~SDLogger() {}
};

// Interface para WiFi
class WiFiManager {
public:
  virtual void init(const char* ssid, const char* password) = 0;
  virtual bool isConnected() = 0;
  virtual const char* getIP() = 0;
  virtual void shutdown() = 0;
  virtual ~WiFiManager() {}
};

// Interface para cliente HTTP (SENSOR)
class HTTP_ClientInterface {
public:
  virtual void init(const char* main_host, int port) = 0;
  virtual bool sendMeasurement(const Measurement& m) = 0;
  virtual bool sendWithRetry(const Measurement& m, int max_retries) = 0;
  virtual void shutdown() = 0;
  virtual ~HTTP_ClientInterface() {}
};

// Interface para servidor web (MAIN)
class Web_ServerInterface {
public:
  virtual void init(int port) = 0;
  virtual void handleRequests() = 0;
  virtual void shutdown() = 0;
  virtual void setSDLogger(SDLogger* logger) = 0;
  virtual ~Web_ServerInterface() {}
};

// Interface para recomendaciones agrícolas
class Recommendations {
public:
  virtual void generateRecommendation(const Measurement& m, char* output, size_t output_size) = 0;
  virtual ~Recommendations() {}
};
