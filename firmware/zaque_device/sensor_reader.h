#pragma once

#include "types.h"

// Interfaz para sensor reader
class SensorReaderImpl : public SensorReader {
public:
  void init() override;
  bool read(Measurement& m) override;
  void shutdown() override;

private:
  void readModbusRegisters(Measurement& m);
  uint8_t calculateBatteryPercent();
};

// Función helper para inicializar el sensor
SensorReader* createSensorReader();
