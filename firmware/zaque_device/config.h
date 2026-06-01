#pragma once

// ============================================================================
// CONFIGURACIÓN DE ROLES
// ============================================================================
#define DEVICE_ROLE_MAIN 1
#define DEVICE_ROLE_SENSOR 2

// Cambiar a DEVICE_ROLE_SENSOR para compilar firmware de sensor
#define DEVICE_ROLE DEVICE_ROLE_MAIN

// ============================================================================
// IDENTIFICACIÓN DEL NODO
// ============================================================================
#if DEVICE_ROLE == DEVICE_ROLE_MAIN
  #define NODE_ID "main"
  #define NODE_NAME "Nodo principal - Casa finca"
#else
  #define NODE_ID "zona_2"
  #define NODE_NAME "Lote de maiz"
#endif

#define FIRMWARE_VERSION "0.2.0"

// ============================================================================
// CONFIGURACIÓN WiFi
// ============================================================================
// NOTA: Para producción, mover a secrets.h
#define WIFI_SSID "Wifi"
#define WIFI_PASSWORD "free6969"
#define WIFI_TIMEOUT_MS 10000

// ============================================================================
// CONFIGURACIÓN DEL NODO MAIN
// ============================================================================
#define MAIN_HOST "zaque.local"
#define MAIN_IP_FALLBACK "192.168.43.50"
#define MAIN_PORT 80

// ============================================================================
// CONFIGURACIÓN DEL NODO SENSOR
// ============================================================================
#define MEASUREMENT_INTERVAL_MINUTES 30
#define SENSOR_SEND_TIMEOUT_MS 5000
#define SENSOR_SEND_MAX_RETRIES 3

// ============================================================================
// PINES - CONSERVADOS DEL PROYECTO ANTERIOR
// ============================================================================
#define GPIO_0_DIVISOR_TENSION 0    // Divisor de tensión para batería (ADC)
#define POWER_PIN 2                 // Control de potencia
#define POWER_PIN2 4                // Control de potencia 2

// ============================================================================
// PINES - SONDA MULTIPARAMÉTRICA RS485/Modbus
// ============================================================================
#define RXD2 16                     // RX del RS485
#define TXD2 17                     // TX del RS485
#define ANALOG_IN_SENSOR A6         // Entrada analógica del sensor

// ============================================================================
// PINES - GPS
// ============================================================================
#define GPIO_TX_GPS 12              // TX del GPS
#define GPIO_RX_GPS 13              // RX del GPS

// ============================================================================
// PINES - TARJETA microSD (SPI)
// ============================================================================
#define GPIO_SD_CS 5                // Chip Select
#define GPIO_SD_SCK 23              // Clock (SPI)
#define GPIO_SD_MOSI 18             // MOSI
#define GPIO_SD_MISO 19             // MISO

// ============================================================================
// CONFIGURACIÓN DE MEDICIÓN
// ============================================================================
#define MEASUREMENT_BUFFER_SIZE 256

// Modbus configuration
#define MODBUS_SLAVE_ADDRESS 1
#define MODBUS_BAUD_RATE 9600

// ============================================================================
// CONFIGURACIÓN DE ALMACENAMIENTO (microSD)
// ============================================================================
#define SD_LOG_DIR "/logs"
#define SD_MEASUREMENTS_FILE "/measurements.csv"
#define SD_LATEST_FILE "/latest.json"
#define SD_CONFIG_FILE "/config.json"
#define SD_NODES_FILE "/nodes.json"

// ============================================================================
// CONFIGURACIÓN DE API
// ============================================================================
#define API_KEY "ZAQUE_LOCAL_KEY"   // NOTA: Mover a secrets.h

// ============================================================================
// SEGURIDAD
// ============================================================================
#define USE_API_KEY true
#define ENABLE_DEBUG_SERIAL true

#if ENABLE_DEBUG_SERIAL
  #define DEBUG_BAUD_RATE 115200
#endif
