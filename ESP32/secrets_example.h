// Copiar este archivo como secrets.h y completar con tus credenciales.
// ¡NUNCA subas secrets.h al repositorio!

// ── WiFi ──────────────────────────────────────────────────────────────
#define WIFI_SSID     "tu_red_wifi"
#define WIFI_PASSWORD "tu_contraseña_wifi"

// ── Broker MQTT ───────────────────────────────────────────────────────
#define MQTT_SERVER   "broker.example.com"
#define MQTT_PORT     1883
#define MQTT_USER     "usuario:vhost"
#define MQTT_PASS     "contraseña_mqtt"

// ── ESP-NOW: MAC del receptor ─────────────────────────────────────────
// Obtener con: Serial.println(WiFi.macAddress());
#define PEER_MAC      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
