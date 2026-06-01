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
    // Inicializar UART2 para comunicación Modbus
    modbus_serial.begin(MODBUS_BAUD_RATE, SERIAL_8N1, RXD2, TXD2);
    
    // Configurar cliente Modbus
    node.begin(MODBUS_SLAVE_ADDRESS, modbus_serial);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);
    
#if ENABLE_DEBUG_SERIAL
    Serial.println("✓ Modbus sensor initialized");
#endif
  }

  bool read(Measurement& m) override {
    // Leer registros Modbus del sensor NPK
    return readModbusRegisters(m);
  }

  void shutdown() override {
    modbus_serial.end();
  }

private:
  static void preTransmission() {
    // Activar transmisión en el módulo RS485
    digitalWrite(POWER_PIN, HIGH);
    delayMicroseconds(100);
  }

  static void postTransmission() {
    // Desactivar transmisión
    delayMicroseconds(100);
    digitalWrite(POWER_PIN, LOW);
  }

  bool readModbusRegisters(Measurement& m) {
    // Variables Modbus
    uint8_t result;
    uint16_t temp_raw, humidity_raw, ec_raw, ph_raw;
    uint16_t nitrogen_raw, phosphorus_raw, potassium_raw;

    // Intentar lectura (reintento 3 veces si falla)
    for (int attempt = 0; attempt < 3; attempt++) {
      // Leer primeros 8 registros del sensor NPK
      result = node.readInputRegisters(0x0000, 8);
      
      if (result == node.ku8MBSuccess) {
        // Parsear valores Modbus (típicamente en registros de 16 bits)
        
        // Temperatura (°C) - Registro 0
        temp_raw = node.getResponseBuffer(0);
        m.soil_temperature = temp_raw / 10.0f;
        
        // Humedad (%) - Registro 1
        humidity_raw = node.getResponseBuffer(1);
        m.soil_humidity = humidity_raw / 10.0f;
        
        // Conductividad (µS/cm) - Registros 2-3
        uint32_t ec_32bit = ((uint32_t)node.getResponseBuffer(2) << 16) | 
                            node.getResponseBuffer(3);
        m.electrical_conductivity = ec_32bit;
        
        // pH - Registro 4
        ph_raw = node.getResponseBuffer(4);
        m.ph = ph_raw / 100.0f;
        
        // Nitrógeno (mg/kg) - Registro 5
        m.nitrogen = node.getResponseBuffer(5);
        
        // Fósforo (mg/kg) - Registro 6
        m.phosphorus = node.getResponseBuffer(6);
        
        // Potasio (mg/kg) - Registro 7
        m.potassium = node.getResponseBuffer(7);
        
        // Leer batería
        m.battery_percent = calculateBatteryPercent();

#if ENABLE_DEBUG_SERIAL
        Serial.printf("✓ Sensor read: T=%.1f H=%.1f pH=%.1f NPK=%d/%d/%d\n",
          m.soil_temperature, m.soil_humidity, m.ph,
          m.nitrogen, m.phosphorus, m.potassium);
#endif
        return true;
      }

#if ENABLE_DEBUG_SERIAL
      Serial.printf("! Modbus read attempt %d failed\n", attempt + 1);
#endif
      delay(100);
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
