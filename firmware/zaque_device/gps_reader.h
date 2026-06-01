#pragma once

#include "types.h"

// Implementación de lectura GPS
class TinyGPSPlusReader : public GPSReader {
public:
  void init() override;
  bool read(float& latitude, float& longitude) override;
  void shutdown() override;

private:
  void readGPSData(float& latitude, float& longitude);
};

// Función helper para inicializar GPS
GPSReader* createGPSReader();
