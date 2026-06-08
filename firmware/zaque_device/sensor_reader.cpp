#include "sensor_reader.h"
#include "config.h"
#include <HardwareSerial.h>
#include <ModbusMaster.h>

// Instancia del objeto Modbus
ModbusMaster node;
static HardwareSerial modbus_serial(2);  // UART2 para RS485

class ModbusSensorReaderImpl : public SensorReaderImpl {
public:
  void init() override {
    // Configurar pines de alimentación
    pinMode(POWER_PIN, OUTPUT);
    pinMode(POWER_PIN2, OUTPUT);
    
    // Enceder alimentación del sensor/módulo
    digitalWrite(POWER_PIN, HIGH);
    digitalWrite(POWER_PIN2, HIGH);
    
    // Esperar a que el sensor se estabilice (como en Prueba_Sensor.ino)
    delay(5000);

    // Inicializar UART2 para comunicación Modbus
    modbus_serial.begin(MODBUS_BAUD_RATE, SERIAL_8N1, RXD2, TXD2);
    
    // Configurar cliente Modbus
    node.begin(MODBUS_SLAVE_ADDRESS, modbus_serial);
    
#if ENABLE_DEBUG_SERIAL
    Serial.println("✓ Modbus sensor initialized and powered");
#endif
  }

  bool read(Measurement& m) override {
    // Leer registros Modbus del sensor NPK
    return readModbusRegisters(m);
  }

  void shutdown() override {
    digitalWrite(POWER_PIN, LOW);
    digitalWrite(POWER_PIN2, LOW);
    modbus_serial.end();
  }

private:
  bool readModbusRegisters(Measurement& m) {
    // Variables Modbus
    uint8_t result;

    // Intentar lectura (reintento 3 veces si falla)
    for (int attempt = 0; attempt < 3; attempt++) {
      // Leer 7 registros Modbus desde la dirección 0x0000 (Holding Registers como en Prueba_Sensor.ino)
      // Registro 0: Humedad
      // Registro 1: Temperatura
      // Registro 2: Conductividad
      // Registro 3: pH
      // Registro 4: Nitrógeno
      // Registro 5: Fósforo
      // Registro 6: Potasio
      result = node.readHoldingRegisters(0x0000, 7);
      
      if (result == node.ku8MBSuccess) {
        // Humedad (%) - Registro 0 (divisor 10)
        m.soil_humidity = node.getResponseBuffer(0) / 10.0f;
        
        // Temperatura (°C) - Registro 1 (divisor 10)
        m.soil_temperature = node.getResponseBuffer(1) / 10.0f;
        
        // Conductividad (µS/cm) - Registro 2
        m.electrical_conductivity = (float)node.getResponseBuffer(2);
        
        // pH - Registro 3 (divisor 10 como en Prueba_Sensor.ino)
        m.ph = node.getResponseBuffer(3) / 10.0f;
        
        // Nitrógeno (mg/kg) - Registro 4
        m.nitrogen = node.getResponseBuffer(4);
        
        // Fósforo (mg/kg) - Registro 5
        m.phosphorus = node.getResponseBuffer(5);
        
        // Potasio (mg/kg) - Registro 6
        m.potassium = node.getResponseBuffer(6);
        
        // Leer batería
        m.battery_percent = calculateBatteryPercent();

#if ENABLE_DEBUG_SERIAL
        Serial.printf("✓ Sensor read: T=%.1f H=%.1f pH=%.1f EC=%.0f NPK=%d/%d/%d\n",
          m.soil_temperature, m.soil_humidity, m.ph, m.electrical_conductivity,
          m.nitrogen, m.phosphorus, m.potassium);
#endif
        return true;
      }

#if ENABLE_DEBUG_SERIAL
      Serial.printf("! Modbus read attempt %d failed (Error: 0x%02X)\n", attempt + 1, result);
#endif
      delay(500); // Un poco más de tiempo entre reintentos
    }

    return false;
  }

  uint8_t calculateBatteryPercent() {
    // Leer ADC del divisor de tensión en GPIO_0_DIVISOR_TENSION
    // ESP32 ADC: 0-4095 = 0-3.3V (con divisor 1:1)
    // Batería típica Li-Ion: 3.0V (0%) a 4.2V (100%)
    
    analogSetAttenuation(ADC_11db);
    uint16_t adc_value = analogRead(GPIO_0_DIVISOR_TENSION);
    
    // Convertir ADC a voltaje
    float voltage = (adc_value / 4095.0f) * 3.3f;
    
    // Mapear 3.0V - 4.2V a 0% - 100%
    float battery_percent = ((voltage - 3.0f) / (4.2f - 3.0f)) * 100.0f;
    
    // Limitar a 0-100
    if (battery_percent < 0) battery_percent = 0;
    if (battery_percent > 100) battery_percent = 100;
    
    return (uint8_t)battery_percent;
  }
};

// Factory function
SensorReader* createSensorReader() {
  return new ModbusSensorReaderImpl();
}
