#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>
#include <ESPmDNS.h>

void ESP32WiFiManager::init(const char* ssid, const char* password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start_time = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start_time < WIFI_TIMEOUT_MS) {
    delay(500);
#if ENABLE_DEBUG_SERIAL
    Serial.print(".");
#endif
  }

  if (WiFi.status() == WL_CONNECTED) {
    strcpy(ip_address, WiFi.localIP().toString().c_str());
    
#if ENABLE_DEBUG_SERIAL
    Serial.println("\n✓ WiFi connected");
    printWiFiStatus();
#endif

    // Inicializar mDNS
    if (!MDNS.begin(NODE_ID)) {
#if ENABLE_DEBUG_SERIAL
      Serial.println("! mDNS init failed");
#endif
    } else {
#if ENABLE_DEBUG_SERIAL
      Serial.printf("✓ mDNS: %s.local\n", NODE_ID);
#endif
    }
  } else {
#if ENABLE_DEBUG_SERIAL
    Serial.println("\n✗ WiFi connection failed");
#endif
  }
}

bool ESP32WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

const char* ESP32WiFiManager::getIP() {
  if (isConnected()) {
    strcpy(ip_address, WiFi.localIP().toString().c_str());
  }
  return ip_address;
}

void ESP32WiFiManager::shutdown() {
  MDNS.end();
  WiFi.disconnect(true);  // true = apagar radio
}

void ESP32WiFiManager::printWiFiStatus() {
#if ENABLE_DEBUG_SERIAL
  Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
  Serial.printf("IP: %s\n", ip_address);
  Serial.printf("Signal: %d dBm\n", WiFi.RSSI());
  Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
#endif
}

// Factory function
WiFiManager* createWiFiManager() {
  return new ESP32WiFiManager();
}
