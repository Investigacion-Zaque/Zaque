#include "gps_reader.h"
#include "config.h"
#include <HardwareSerial.h>
#include <TinyGPS++.h>

static HardwareSerial gps_serial(1);  // UART1 para GPS
static TinyGPSPlus gps;

class TinyGPSPlusReaderImpl : public TinyGPSPlusReader {
public:
  void init() override {
    // Inicializar UART1 para GPS
    gps_serial.begin(9600, SERIAL_8N1, GPIO_RX_GPS, GPIO_TX_GPS);

#if ENABLE_DEBUG_SERIAL
    Serial.println("✓ GPS reader initialized (9600 bps)");
#endif
  }

  bool read(float& latitude, float& longitude) override {
    return readGPSData(latitude, longitude);
  }

  void shutdown() override {
    gps_serial.end();
  }

private:
  bool readGPSData(float& latitude, float& longitude) {
    // Leer datos GPS durante 1 segundo
    unsigned long start = millis();
    while (millis() - start < 1000) {
      while (gps_serial.available()) {
        gps.encode(gps_serial.read());
      }
    }

    // Verificar si tenemos ubicación válida
    if (gps.location.isValid()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();

#if ENABLE_DEBUG_SERIAL
      Serial.printf("✓ GPS fix: %.6f, %.6f (Sats: %d)\n",
        latitude, longitude, gps.satellites.value());
#endif
      return true;
    }

#if ENABLE_DEBUG_SERIAL
    Serial.println("! GPS no valid location yet");
#endif
    return false;
  }
};

// Factory function
GPSReader* createGPSReader() {
  return new TinyGPSPlusReaderImpl();
}
