#pragma once

#include "types.h"

// Implementación de logging en microSD
class SDCardLogger : public SDLogger {
public:
  void init() override;
  bool logMeasurement(const Measurement& m) override;
  bool readLatest(Measurement& m) override;
  void shutdown() override;

private:
  bool ensureDirectoriesExist();
  void measurementToCSV(const Measurement& m, char* buffer, size_t size);
  void measurementToJSON(const Measurement& m, char* buffer, size_t size);
};

// Función helper
SDLogger* createSDLogger();
