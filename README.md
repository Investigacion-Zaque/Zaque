# Zaque 🌾

**Sistema local de monitoreo agrícola distribuido para comunidades campesinas.**

Zaque es una red de dispositivos ESP32 que miden variables del suelo en múltiples puntos de una finca, almacenan datos localmente en microSD y exponen un dashboard web desde el nodo principal, **sin dependencia de internet**.

El campesino enciende la zona móvil del celular, los ESP32 se conectan a esa red WiFi local, y puede visualizar todas las mediciones desde el navegador del teléfono.

---

## 🎯 Visión

Transformar Zaque de una arquitectura basada en **MQTT hacia la nube** a una arquitectura **local, autónoma y tolerante a falta de internet**, priorizando el uso en zonas rurales sin conectividad confiable.

---

## 🏗️ Arquitectura Nueva

```
┌──────────────────────────────────────┐
│  Red WiFi local del celular          │
└──────────────────────────────────────┘
          ▲          ▲          ▲
          │          │          │
    ┌─────┴────┐ ┌──┴────┐ ┌───┴─────┐
    │ SENSOR 1 │ │SENSOR 2│ │SENSOR N │
    │  GPS+SD  │ │ GPS+SD │ │ GPS+SD  │
    └─────┬────┘ └──┬─────┘ └───┬─────┘
          │HTTP POST │          │
          └──────┬───┴──────────┘
               ▼
    ┌──────────────────────┐
    │ MAIN (Web + API)     │
    │ • Sensor local       │
    │ • GPS local          │
    │ • microSD (central)  │
    │ • Dashboard local    │
    │ • Recomendaciones    │
    └──────────┬───────────┘
          HTTP ▼
      ┌─────────────────┐
      │ Celular         │
      │ Navegador web   │
      └─────────────────┘
```

**Componentes**:
- **1 nodo MAIN**: Punto central, servidor web, almacenamiento
- **N nodos SENSOR**: Puntos distribuidos, envían datos al MAIN
- **Mismo hardware para todos**: Diferencia solo en firmware/configuración
- **Red WiFi local**: Creada por el celular (hotspot)
- **Almacenamiento local**: Todos los datos en microSD
- **Dashboard embebido**: Página web servida desde el MAIN

---

## 📋 Estado del Proyecto

| Fase | Estado | Descripción |
|------|--------|-------------|
| **0. Migración de arquitectura** | ✅ **COMPLETADA** | Nueva estructura de carpetas, headers, firmware base |
| **1. MAIN autónomo** | 🔜 En progreso | Lectura de sensor, GPS, SD, servidor web básico |
| **2. API local del MAIN** | 🔜 Próximo | Endpoint POST para recibir mediciones de sensores |
| **3. Firmware SENSOR** | 🔜 Próximo | WiFi, medición, envío HTTP, deep sleep |
| **4. Múltiples sensores** | 🔜 Próximo | Escalabilidad a 3+ nodos |
| **5. Recomendaciones** | 🔜 Próximo | Motor de reglas agrícolas simples |
| **6. Descubrimiento automático** | 🔜 Futuro | mDNS, UDP discovery |
| **7. Sincronización con nube** | 🔜 Futuro | MQTT/API remota opcional |

---

## 📁 Estructura del Repositorio

```
Zaque/
├── firmware/
│   ├── zaque_device/                # Firmware unificado nuevo
│   │   ├── zaque_device.ino         # Archivo principal
│   │   ├── config.h                 # Configuración roles, pines, WiFi
│   │   ├── types.h                  # Estructuras de datos y interfaces
│   │   ├── sensor_reader.h           # Lectura de sensor RS485/Modbus
│   │   ├── gps_reader.h             # Lectura de GPS
│   │   ├── sd_logger.h              # Logging en microSD
│   │   ├── wifi_manager.h           # Gestión WiFi
│   │   ├── http_client.h            # Cliente HTTP (SENSOR)
│   │   ├── web_server.h             # Servidor web (MAIN)
│   │   ├── node_registry.h          # Registro de nodos (MAIN)
│   │   ├── recommendations.h        # Engine de recomendaciones
│   │   └── dashboard_html.h         # HTML/CSS/JS embebido
│   │
│   └── legacy/                       # Código anterior (referencia)
│       ├── ESPNOW_bajo_consumo_emisor.ino
│       └── Receptor_con_MQTT_y_Bateria.ino
│
├── docs/
│   ├── arquitectura_offline_multinodo.md    # Descripción de arquitectura
│   ├── pinout.md                    # Asignación de pines ESP32
│   ├── protocolo_api_local.md       # Especificación de API HTTP
│   ├── hardware.md                  # Recomendaciones de hardware
│   ├── formato_sd.md                # Estructura de archivos en SD
│   └── plan_migracion.md            # Plan de implementación por fases
│
├── dashboard/
│   ├── prototype.html               # Prototipo visual (futuro)
│   ├── styles.css                   # Estilos (futuro)
│   └── app.js                       # Lógica JS (futuro)
│
├── tools/
│   ├── csv_to_json.py               # Herramienta de conversión
│   └── sd_validator.py              # Validador de archivos SD
│
├── cloud_optional/                  # Sincronización a nube (opcional)
│   ├── mqtt_bridge/                 # Bridge MQTT (migración futura)
│   ├── api/                         # API remota (futuro)
│   └── database/                    # BD remota (futuro)
│
├── Base de datos/                   # (Anterior) Node.js + MQTT
├── Backend/                         # (Anterior) Backend futuro
├── Frontend/                        # (Anterior) Frontend futuro
├── Api/                             # (Anterior) API futura
├── ESP32/                           # (Anterior) Código legado
│
├── README.md                        # Este archivo
├── LICENSE                          # Licencia
└── PIN_OUT.txt                      # Configuración de pines (referencia)
```

---

## 🔌 Hardware Recomendado

**Idéntico para MAIN y SENSOR:**

| Componente | Cantidad | Especificación |
|-----------|----------|-----------------|
| ESP32 DEVKIT | 1 | Microcontrolador WiFi |
| Sensor RS485 | 1 | NPK/Modbus (temperatura, humedad, pH, NPK) |
| Módulo GPS | 1 | NEO-6M o similar (9600 bps) |
| microSD | 1 | 32GB (almacenamiento local) |
| Batería | 1 | 3.7V Li-Ion con protección BMS |
| Regulador | 1 | DC-DC 5V → 3.3V |

**Pines ESP32** (ver `/docs/pinout.md`):
- GPIO 0: ADC batería
- GPIO 2, 4: Control de potencia
- GPIO 16/17: UART2 (RS485 Modbus)
- GPIO 12/13: UART1 (GPS) ⚠️ Conflicto con SPI
- GPIO 5, 18, 19: SPI (microSD)
- A6: Entrada analógica sensor

⚠️ **Nota de conflicto**: GPIO 13 es compartido entre GPS y SD. Ver `/docs/pinout.md` para soluciones.

---

## 🚀 Inicio Rápido

### 1. Compilar Firmware MAIN

```bash
# Arduino IDE:
# - Abrir: firmware/zaque_device/zaque_device.ino
# - Board: ESP32 Dev Module
# - Verificar config.h:
#   - DEVICE_ROLE = DEVICE_ROLE_MAIN
#   - WIFI_SSID, WIFI_PASSWORD = tu zona móvil
#   - NODE_ID, NODE_NAME = identificar nodo
# - Subir sketch
```

### 2. Compilar Firmware SENSOR

```bash
# En config.h, cambiar:
#   DEVICE_ROLE = DEVICE_ROLE_SENSOR
#   NODE_ID = "zona_2"
#   NODE_NAME = "Lote de maiz"
# Subir a otro ESP32
```

### 3. Ejecutar

```bash
1. Enciende zona móvil del celular
2. Enciende ESP32 MAIN
   - Conecta a WiFi
   - Levanta server en http://zaque.local
3. Enciende ESP32 SENSOR 1, 2, ... N
   - Conectan a WiFi
   - Toman medición
   - Envían a MAIN
   - Entran en deep sleep
4. Abre navegador en celular
   - Entra a http://zaque.local
   - Ve dashboard con todos los nodos
```

---

## 📊 Mediciones

| Variable | Unidad | Sensor |
|----------|--------|--------|
| Temperatura suelo | °C | RS485/Modbus |
| Humedad suelo | % | RS485/Modbus |
| Conductividad | µS/cm | RS485/Modbus |
| pH | — | RS485/Modbus |
| Nitrógeno (N) | mg/kg | RS485/Modbus |
| Fósforo (P) | mg/kg | RS485/Modbus |
| Potasio (K) | mg/kg | RS485/Modbus |
| Ubicación | lat/lon | GPS NEO-6M |
| Batería | % | ADC ESP32 |

---

## 📡 API Local

**Endpoints del MAIN:**

| Método | Ruta | Descripción |
|--------|------|-------------|
| GET | `/` | Dashboard HTML |
| GET | `/api/status` | Estado del sistema |
| GET | `/api/nodes` | Lista de nodos |
| GET | `/api/measurements/latest` | Última medición por nodo |
| GET | `/api/history?node_id=zona_2` | Histórico de un nodo |
| POST | `/api/measurements` | Recibir medición (SENSOR → MAIN) |
| GET | `/download/measurements.csv` | Descargar CSV |

**Ejemplo - Enviar medición (curl):**

```bash
curl -X POST http://zaque.local/api/measurements \
  -H "Content-Type: application/json" \
  -d '{
    "api_key": "ZAQUE_LOCAL_KEY",
    "node_id": "zona_2",
    "node_name": "Lote de maiz",
    "role": "sensor",
    "timestamp": "2026-05-31T16:20:00",
    "lat": 4.7112,
    "lon": -74.0721,
    "soil_humidity": 64.0,
    "ph": 6.2,
    "nitrogen": 34,
    "phosphorus": 18,
    "potassium": 41,
    "battery_percent": 83,
    "firmware_version": "0.2.0"
  }'
```

Ver `/docs/protocolo_api_local.md` para documentación completa.

---

## 💾 Almacenamiento en microSD

**MAIN**:
```
/sd/
├── config.json          # Configuración del nodo
├── nodes.json           # Registry de nodos
├── latest.json          # Última medición (para dashboard)
├── measurements.csv     # Histórico CSV
└── logs/system.log      # Eventos del sistema
```

**SENSOR**:
```
/sd/
├── config.json          # Configuración local
├── local_measurements.csv  # Mediciones locales
├── pending_queue.csv    # Cola de envíos pendientes
└── logs/sensor.log      # Eventos del sensor
```

---

## 🌱 Recomendaciones Agrícolas

Motor simple basado en reglas (futuro: expandible):

```
- Humedad < 30% → "Revisar riego"
- pH < 5.5      → "Suelo ácido, considerar enmienda"
- pH > 7.5      → "Suelo alcalino, revisar fertilizante"
- NPK < 30      → "Revisar fertilización"
- Batería < 20% → "Cargar nodo"
```

Aviso importante: Las recomendaciones son orientativas y deben ajustarse según cultivo y acompañamiento técnico local.

---

## 🔋 Bajo Consumo

**SENSOR típico:**
- Despierta cada 30 minutos
- Mide en ~5 segundos
- Envía en ~2 segundos
- Duerme el 99% del tiempo
- **Autonomía**: 1-3 meses con batería estándar

---

## 📚 Documentación

- **`/docs/arquitectura_offline_multinodo.md`** – Descripción general de la nueva arquitectura
- **`/docs/pinout.md`** – Asignación de pines y conflictos
- **`/docs/protocolo_api_local.md`** – Especificación de endpoints HTTP
- **`/docs/hardware.md`** – Recomendaciones de componentes
- **`/docs/formato_sd.md`** – Estructura de archivos en SD
- **`/informe_migracion_zaque_offline_multinodo.md`** – Informe técnico completo

---

## 🔐 Seguridad

**MVP (actual):**
- API key simple en POST (por cambiar antes de compilar)
- Sin HTTPS (red local no requiere)
- Validación JSON básica

**Futuro:**
- JWT tokens
- HTTPS cuando haya conectividad
- Autenticación de nodos

---

## ⚡ Próximos Pasos

1. ✅ [HECHO] Arquitectura y documentación
2. 🔜 Fase 1: MAIN autónomo con dashboard
3. 🔜 Fase 2: API local para sensores
4. 🔜 Fase 3: Firmware SENSOR completo
5. 🔜 Fase 4: Escalabilidad a múltiples sensores
6. 🔜 Fase 5: Recomendaciones agrícolas
7. 🔜 Fase 6: Descubrimiento automático (mDNS)
8. 🔜 Fase 7: Sincronización opcional con nube

---

## 📖 Modo Online (Antiguo)

La arquitectura anterior basada en MQTT se conserva en:
- `firmware/legacy/` – Código antiguo
- `cloud_optional/` – Futuro: sincronización con servidor

Esta versión seguirá siendo soportada como modo remoto opcional cuando haya conectividad a internet.

---

## 📄 Licencia

Ver [LICENSE](LICENSE)

---

**Zaque**: Monitoreo agrícola local, autónomo, tolerante a falta de internet. Hecho para campesinos, por software libre.

