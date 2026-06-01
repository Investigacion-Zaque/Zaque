# Pinout - Zaque Offline Multinodo

## Resumen

Cada nodo Zaque usa un ESP32 con múltiples módulos externos. Este documento detalla la asignación de pines para toda la arquitectura.

## Diagrama General

```
┌─────────────────────────────────────┐
│          ESP32 DEVKIT               │
│                                     │
│  GPIO 0  ──→ Divisor de tensión     │
│  GPIO 2  ──→ Power control 1        │
│  GPIO 4  ──→ Power control 2        │
│  GPIO 5  ──→ SD CS (SPI)            │
│  GPIO 12 ──→ GPS TX                 │
│  GPIO 13 ──→ GPS RX                 │
│  GPIO 16 ──→ RS485 RX (UART2)       │
│  GPIO 17 ──→ RS485 TX (UART2)       │
│  GPIO 18 ──→ SD MOSI (SPI)          │
│  GPIO 19 ──→ SD MISO (SPI)          │
│  GPIO 23 ──→ SD SCK (SPI)           │
│  A6      ──→ Analog sensor input    │
│                                     │
│ GND/3V3 connections (ver abajo)     │
└─────────────────────────────────────┘
```

## Pines Utilizados

### 1. Control de Potencia (Conservado)

| Pin | Nombre        | Propósito                          | Voltaje | Notas                |
|-----|---------------|-----------------------------------|---------|----------------------|
| 0   | GPIO_0        | Divisor de tensión (ADC)          | 0-3.3V  | Monitoreo de batería |
| 2   | POWER_PIN     | Control de alimentación módulos   | 3.3V    | Salida digital       |
| 4   | POWER_PIN2    | Control de alimentación módulos   | 3.3V    | Salida digital       |

### 2. Sensor de Suelo RS485/Modbus (Conservado)

| Pin | Nombre  | Propósito              | Voltaje | Protocolo | Notas           |
|-----|---------|------------------------|---------|-----------|-----------------|
| 16  | RXD2    | RX UART2 (RS485)       | 3.3V    | UART/RS485| Modbus slave    |
| 17  | TXD2    | TX UART2 (RS485)       | 3.3V    | UART/RS485| Modbus slave    |
| A6  | ANALOG  | Entrada analógica      | 0-3.3V  | ADC       | Sensor secundario |

### 3. Módulo GPS (Nuevo)

| Pin | Nombre      | Propósito              | Voltaje | Protocolo | Notas          |
|-----|-------------|------------------------|---------|-----------|----------------|
| 12  | GPIO_TX_GPS | TX UART1 (GPS RX)      | 3.3V    | UART 9600bps | GPS entrada    |
| 13  | GPIO_RX_GPS | RX UART1 (GPS TX)      | 3.3V    | UART 9600bps | GPS salida     |

### 4. Tarjeta microSD (Nuevo)

| Pin | Nombre    | Propósito        | Voltaje | Protocolo | Notas           |
|-----|-----------|------------------|---------|-----------|-----------------|
| 5   | GPIO_CS   | Chip Select (SPI) | 3.3V    | SPI      | Slave select    |
| 18  | GPIO_MOSI | MOSI (SPI)       | 3.3V    | SPI      | Master out      |
| 19  | GPIO_MISO | MISO (SPI)       | 3.3V    | SPI      | Master in       |
| 23  | GPIO_SCK  | Clock (SPI)      | 3.3V    | SPI      | Clock           |


### Conflicto GPIO 13: RESUELTO

**Solución implementada**:
- GPS RX: GPIO 13 ✓
- SD SCK: GPIO 23 ✓

Ambas interfaces funcionan sin conflicto.

## Pines Disponibles / No Utilizados

| Pin | Estado | Notas |
|-----|--------|-------|
| 1   | Libre  | TX serial debug |
| 3   | Libre  | RX serial debug |
| 6-11 | Libres | Disponibles para expansión |
| 14  | Libre  | Disponible |
| 15  | Libre  | Disponible |
| 21-22 | Libres | Disponibles |
| 24-27 | Libres | Disponibles |
| 32-39 | Libres | GPIO32-39 disponibles (algunos para ADC) |

## Alimentación

### Voltajes

- **ESP32**: 3.3V desde regulador
- **Sensor RS485**: Depende del módulo (usualmente 5V con conversor)
- **GPS**: Típicamente 3.3V o 5V (verificar módulo)
- **microSD**: 3.3V
- **ADC batería**: 0-3.3V (incluye divisor de tensión en GPIO 0)

### GND

Todos los módulos deben compartir GND (plano de tierra común).

## Diagrama de Conexión

```
BATERÍA ────┬──→ 5V Regulador ──→ [+5V bus]
            └──→ Divisor (GPIO0) ──→ ADC
                                       
[+5V bus] ──→ Sensor RS485 (vcc)
[+3.3V] ──→ GPS (vcc si 3.3V)
[+3.3V] ──→ microSD (vcc)
[GND] ─────→ Todos los módulos

RS485 ────┬─→ RX GPIO16 (UART2)
          └─→ TX GPIO17 (UART2)

GPS ──────┬─→ RX GPIO13
          └─→ TX GPIO12

microSD ──┬─→ CS GPIO5
          ├─→ SCK GPIO23
          ├─→ MOSI GPIO18
          └─→ MISO GPIO19
```

## Recomendaciones

1. **Hardware**: Reservar soldadura fría para pines 10-12 como alternativa GPS
2. **Testing**: Validar lecturas simultáneas de RS485 y SD
3. **Documentación**: Crear guía de ensamblado con fotos de conexiones

## Referencias

- ESP32 Datasheet: Pin assignments y capacidades
- RS485 Modbus: Protocolo de comunicación con sensor de suelo
- GPS: TinyGPS+ library requerida
- microSD: Librería SD estándar de Arduino
