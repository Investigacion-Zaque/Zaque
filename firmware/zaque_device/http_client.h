#pragma once

#include "types.h"
#include <HTTPClient.h>

// Implementación de cliente HTTP para envío de datos desde SENSOR al MAIN
class ESP32HTTPClient : public HTTP_ClientInterface {
public:
  void init(const char* main_host, int port) override;
  bool sendMeasurement(const Measurement& m) override;
  bool sendWithRetry(const Measurement& m, int max_retries) override;
  void shutdown() override;

private:
  char main_host[256];
  int main_port;
  char main_url[512];
  ::HTTPClient http_client;  // Usar HTTPClient de la librería con namespace
  
  bool buildPayloadJSON(const Measurement& m, char* buffer, size_t size);
  bool parseResponse(const char* response, char* recommendation, size_t rec_size);
};

// Función helper
HTTP_ClientInterface* createHTTPClient();
