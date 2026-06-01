#pragma once

#include "types.h"
#include "sd_logger.h"
#include <WebServer.h>

// Implementación de servidor web embebido para el MAIN
class ESP32WebServer : public Web_ServerInterface {
public:
  void init(int port) override;
  void handleRequests() override;
  void shutdown() override;
  void setSDLogger(SDLogger* logger) override;

  ::WebServer server;
  int server_port;
  SDLogger* sd_logger;
  
  ESP32WebServer(int port) : server(port), server_port(port), sd_logger(nullptr) {}
  
  void handleRoot();
  void handleStatusAPI();
  void handleNodesAPI();
  void handleLatestAPI();
  void handleHistoryAPI();
  void handleMeasurementsAPI();
  void handleDownloadCSV();
};

// Función helper
Web_ServerInterface* createWebServer();
