#pragma once

#include "types.h"

// Implementación de WiFi ESP32
class ESP32WiFiManager : public WiFiManager {
public:
  void init(const char* ssid, const char* password) override;
  bool isConnected() override;
  const char* getIP() override;
  void shutdown() override;

protected:
  char ip_address[16];
  void printWiFiStatus();
};

// Función helper
WiFiManager* createWiFiManager();
