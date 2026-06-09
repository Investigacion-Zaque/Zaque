# 🌾 Zaque - Monitoreo Agrícola Local Distribuido

> **Sistema de monitoreo de suelo para campesinos con IoT, sin dependencia de internet**

---

## 🎯 ¿Qué es Zaque?

Zaque es una **red de dispositivos ESP32 que monitorean cultivos** en múltiples puntos de una finca. El campesino enciende la zona móvil del celular, los ESP32 se conectan a esa red local, y puede ver todas las mediciones desde su navegador móvil **sin necesidad de internet**.

| Característica | Ventaja |
|---|---|
| 📡 **WiFi Local** | Sin necesidad de internet |
| 💾 **Almacenamiento Local** | Datos en microSD dentro del dispositivo |
| 📍 **GPS en cada nodo** | Sabe dónde se midió cada dato |
| 📊 **Dashboard web** | Visualiza desde el navegador del celular |
| 🤖 **Recomendaciones inteligentes** | Alertas de riego, fertilización, pH |
| ⚡ **Bajo consumo** | 1-3 meses de batería por nodo |
| 💰 **Bajo costo** | Hardware accesible (~USD $100 por nodo) |

---

## 🏗️ Arquitectura

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃   Red WiFi Local del Celular 📱         ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
    △           △           △           △
    │           │           │           │
    │           │           │           │
┌───▼──┐    ┌───▼──┐    ┌───▼──┐    ┌─MAIN─┐
│SENSOR│    │SENSOR│    │SENSOR│    │MAIN  │
│  #1  │    │  #2  │    │  #3  │    │NODE  │
│      │    │      │    │      │    │      │
│📍GPS │    │📍GPS │    │📍GPS │    │📍GPS │
│📊SD  │    │📊SD  │    │📊SD  │    │📊SD  │
│🔋BAT │    │🔋BAT │    │🔋BAT │    │🔋BAT │
└───┬──┘    └───┬──┘    └───┬──┘    └─┬────┘
    │ HTTP       │ HTTP       │ HTTP    │ Servidor
    │ POST       │ POST       │ POST    │ Web
    └────────────┼────────────┼────────┘
                 │
                 ▼
        ┏━━━━━━━━━━━━━━━━━┓
        ┃  📊 Dashboard   ┃
        ┃  en Navegador   ┃
        ┃  (celular)      ┃
        ┗━━━━━━━━━━━━━━━━━┛
```

**Componentes:**

- **1 nodo MAIN**: Centro de procesamiento, servidor web, almacenamiento
- **N nodos SENSOR**: Distribuidos en la finca, envían mediciones
- **Red local WiFi**: Creada por el hotspot del celular
- **Almacenamiento**: Todos los datos en microSD local (sin nube)

---

## 📋 Estado del Proyecto (v0.2.0)

| Componente | Estado | Descripción |
|---|---|---|
| **Arquitectura** | ✅ Completado | Migración a local offline |
| **Firmware base** | ✅ Completado | zaque_device.ino con ambos roles |
| **Lectura de sensores** | ✅ Completado | RS485 Modbus (humedad, temp, pH, NPK) |
| **GPS** | ✅ Completado | Fallback a ubicación por defecto |
| **SD Logger** | ✅ Completado | CSV + JSON local |
| **WiFi Manager** | ✅ Completado | Conexión a redes locales |
| **Recomendaciones** | ✅ Completado | Motor de reglas de cultivo |
| **API REST** | 🔜 En progreso | Endpoints POST/GET |
| **Dashboard Web** | 🔜 Próximo | HTML embebido, mapas, históricos |
| **Múltiples sensores** | 🔜 Próximo | Escalabilidad 3+ nodos |
| **Sincronización nube** | 🔜 Futuro | MQTT/API remota opcional |

---

## 📊 Variables Medidas

| Variable | Unidad | Sensor | Rango |
|---|---|---|---|
| **Humedad del suelo** | % | RS485 Modbus | 0-100% |
| **Temperatura del suelo** | °C | RS485 Modbus | -10 a +60°C |
| **Conductividad eléctrica** | mS/cm | RS485 Modbus | 0-10 |
| **Potencial de hidrógeno (pH)** | pH | RS485 Modbus | 3.0-9.0 |
| **Nitrógeno (N)** | mg/kg | RS485 Modbus | 0-999 |
| **Fósforo (P)** | mg/kg | RS485 Modbus | 0-999 |
| **Potasio (K)** | mg/kg | RS485 Modbus | 0-999 |
| **Ubicación** | lat/lon | GPS NEO-6M | - |
| **Nivel de batería** | % | ADC ESP32 | 0-100% |

---

## 🤖 Recomendaciones Inteligentes

El sistema genera **alertas útiles al campesino** basadas en mediciones:

### 🔴 Alertas Críticas
- **Humedad < 25%**: "Riesgo de estrés hídrico severo. Revise riego lo antes posible."
- **Humedad > 85%**: "Riesgo de encharcamiento. Suspenda riego y revise drenaje."
- **Conductividad > 4.0 mS/cm**: "Posible exceso de sales. Suspenda fertilización."

### 🟠 Problemas Moderados
- **Humedad baja + Temperatura alta**: "Estrés hídrico. Revise riego urgentemente."
- **pH ácido + Fósforo bajo**: "Bloqueo nutricional. Revise corrección antes de fertilizar."
- **Humedad alta + Conductividad alta**: "Acumulación de sales. Evite fertilizar."

### 🟢 Recomendaciones Específicas
- **NPK bajo**: "Revise estado del cultivo y considere fertilización."
- **pH fuera de rango**: "Suelo ácido/alcalino. Consulte técnico para corrección."

### ✅ Estado Óptimo
- Cuando todo está bien: "Condiciones óptimas: Mantenga monitoreo."

---

## 🔌 Hardware Recomendado

**Idéntico para MAIN y SENSOR:**

```
┌─────────────────────────────────┐
│ ESP32 DevKit (WiFi + Bluetooth) │
├─────────────────────────────────┤
│ • Microcontrolador dual-core     │
│ • WiFi 802.11b/g/n              │
│ • 4 MB Flash + 520 KB RAM        │
│ • 36 pines GPIO                  │
└─────────────────────────────────┘
         │
    ┌────┴────┐
    │          │
    ▼          ▼
┌─────────┐ ┌─────────────────┐
│ Sensor  │ │ GPS + microSD   │
│ Modbus  │ │ + Batería       │
│ RS485   │ │ + Regulador     │
└─────────┘ └─────────────────┘
```

| Componente | Cantidad | Especificación | Costo |
|---|---|---|---|
| ESP32 DEVKIT | 1 | Microcontrolador WiFi | ~$15 |
| Sensor RS485 | 1 | NPK/Modbus multiparamétrico | ~$50 |
| Módulo GPS | 1 | NEO-6M (9600 bps) | ~$12 |
| microSD | 1 | 32GB FAT32 | ~$8 |
| Batería Li-Ion | 1 | 3.7V 2000mAh con BMS | ~$10 |
| Regulador DC-DC | 1 | 5V → 3.3V | ~$3 |
| **Total por nodo** | - | - | **~$100** |

---

## 📁 Estructura del Repositorio

```
Zaque/
│
├── 📦 firmware/
│   ├── zaque_device/              ← FIRMWARE UNIFICADO (MAIN/SENSOR)
│   │   ├── zaque_device.ino       ← Archivo principal
│   │   ├── config.h               ← Configuración (roles, pines, WiFi)
│   │   ├── types.h                ← Estructuras de datos e interfaces
│   │   ├── sensor_reader.cpp      ← Lectura RS485 Modbus
│   │   ├── gps_reader.cpp         ← Lectura GPS + fallback
│   │   ├── sd_logger.cpp          ← Logging en microSD
│   │   ├── wifi_manager.cpp       ← Gestión WiFi local
│   │   ├── http_client.cpp        ← Cliente HTTP (SENSOR)
│   │   ├── web_server.cpp         ← Servidor web (MAIN)
│   │   ├── node_registry.cpp      ← Registro de nodos (MAIN)
│   │   ├── recommendations.cpp    ← Motor de recomendaciones
│   │   └── dashboard_html.h       ← HTML/CSS/JS embebido
│   │
│   ├── Prueba_SD/                 ← Test de tarjeta SD
│   ├── Prueba_Sensor/             ← Test de sensor Modbus
│   └── legacy/                    ← Código anterior (referencia)
│
├── 📚 docs/
│   ├── arquitectura_offline_multinodo.md
│   ├── pinout.md
│   ├── protocolo_api_local.md
│   ├── hardware.md
│   ├── formato_sd.md
│   └── plan_migracion.md
│
├── 📄 Articulo/                   ← Artículo académico LaTeX
│   └── main.tex
│
├── 📋 README.md                   ← Este archivo
├── 📋 GUIA_FLASHEO_Y_USO.md       ← Guía de instalación
├── 📋 ZAQUE_RECOMMENDATION_ENGINE.md
├── 📋 informe_migracion_zaque_offline_multinodo.md
├── 📋 PIN_OUT.txt
└── 📋 LICENSE
```

---

## 🚀 Inicio Rápido

### 1️⃣ Configurar Firmware MAIN

```cpp
// En config.h:
#define DEVICE_ROLE DEVICE_ROLE_MAIN        // ✓ MAIN
#define NODE_ID "zaque"
#define NODE_NAME "Nodo principal - Casa finca"
#define WIFI_SSID "Zaque"                   // Tu zona móvil
#define WIFI_PASSWORD "free12345"
#define DEFAULT_LATITUDE 4.6632              // Ubicación por defecto (GPS fallback)
#define DEFAULT_LONGITUDE -74.0550
```

**En Arduino IDE:**
- Abrir: `firmware/zaque_device/zaque_device.ino`
- Seleccionar: Board → ESP32 Dev Module
- Puerto: COM/ttyUSB
- Subir

### 2️⃣ Configurar Firmware SENSOR

```cpp
// En config.h:
#define DEVICE_ROLE DEVICE_ROLE_SENSOR      // ✓ SENSOR
#define NODE_ID "zona_2"
#define NODE_NAME "Lote de maiz"
#define MAIN_HOST "zaque.local"             // IP/hostname del MAIN
```

Cambiar y subir a otro ESP32.

### 3️⃣ Ejecutar en Campo

```
1. Enciende zona móvil del celular ("Zaque")
2. Enciende ESP32 MAIN
   └─→ Se conecta a WiFi
   └─→ Levanta servidor en http://zaque.local
   └─→ Toma medición local
   └─→ Espera sensores
3. Enciende ESP32 SENSOR(es)
   └─→ Se conectan a WiFi
   └─→ Toman mediciones
   └─→ Envían al MAIN por HTTP POST
   └─→ Entran en deep sleep (ahorro energía)
4. Abre navegador en celular
   └─→ Entra a http://zaque.local
   └─→ Ve dashboard con todos los nodos
   └─→ Lee recomendaciones
```

---

## 📡 API REST Local

### Endpoints del MAIN

| Método | Ruta | Descripción |
|---|---|---|
| **GET** | `/` | Dashboard HTML interactivo |
| **GET** | `/api/status` | Estado general del sistema |
| **GET** | `/api/nodes` | Lista de nodos activos |
| **GET** | `/api/latest` | Última medición por nodo |
| **POST** | `/api/measurements` | Recibir medición (SENSOR → MAIN) |
| **GET** | `/download/measurements.csv` | Descargar histórico |

### Ejemplo: Enviar Medición

```bash
curl -X POST http://zaque.local/api/measurements \
  -H "Content-Type: application/json" \
  -d '{
    "api_key": "ZAQUE_LOCAL_KEY",
    "node_id": "zona_2",
    "node_name": "Lote de maiz",
    "latitude": 4.6632,
    "longitude": -74.0550,
    "soil_humidity": 64.0,
    "soil_temperature": 22.5,
    "electrical_conductivity": 1.2,
    "ph": 6.2,
    "nitrogen": 34,
    "phosphorus": 18,
    "potassium": 41,
    "battery_percent": 83
  }'
```

---

## 💾 Almacenamiento en microSD

**Archivos creados automáticamente:**

```
/measurements.csv
  timestamp,node_id,node_name,lat,lon,soil_temperature,soil_humidity,...
  1715609400,zona_2,Lote de maiz,4.6632,-74.0550,22.5,64.0,...

/latest.json
  {
    "updated_at": "1715609400",
    "nodes": [
      {
        "node_id": "zona_2",
        "soil_humidity": 64.0,
        "ph": 6.2,
        "recommendation": "✅ Condiciones óptimas: Mantenga monitoreo."
      }
    ]
  }

/logs/system.log
  [DEBUG] GPS reader initialized
  [INFO] Measurement logged to SD
  [WARNING] Battery low: 15%
```

---

## ⚡ Consumo de Energía

### Nodo SENSOR típico
```
Medición:      ~5 seg
Envío HTTP:    ~2 seg
Deep Sleep:    ~30 min

Consumo promedio: ~0.5 mA (en deep sleep)
Batería típica:   2000 mAh
Autonomía:        ≈ 1-3 meses
```

### Nodo MAIN (siempre activo)
```
WiFi siempre conectado
Dashboard activo
Consumo: ~80-150 mA promedio

Nota: MAIN debe estar conectado a alimentación (AC/solar)
```

---

## 🔐 Seguridad

| Aspecto | MVP (Actual) | Futuro |
|---|---|---|
| **Red** | WiFi local | WiFi local + VPN opcional |
| **API Key** | Hardcodeada | JWT tokens |
| **HTTPS** | No necesario | Sí, cuando haya internet |
| **Autenticación** | Ninguna | Por nodo |
| **Datos** | Almacenamiento local | Encriptación opcional |

---

## 📚 Documentación Completa

| Documento | Contenido |
|---|---|
| 📖 **`docs/arquitectura_offline_multinodo.md`** | Detalles arquitectura distribuida |
| 🔌 **`docs/pinout.md`** | Asignación de pines ESP32 y conflictos |
| 📡 **`docs/protocolo_api_local.md`** | Especificación completa de API |
| 🔧 **`docs/hardware.md`** | Recomendaciones y esquemáticos |
| 💾 **`docs/formato_sd.md`** | Estructura de archivos SD |
| 📋 **`docs/plan_migracion.md`** | Fases de implementación |
| 🤖 **`ZAQUE_RECOMMENDATION_ENGINE.md`** | Motor de recomendaciones |
| 📊 **`informe_migracion_zaque_offline_multinodo.md`** | Informe técnico completo |
| 🚀 **`GUIA_FLASHEO_Y_USO.md`** | Paso a paso de instalación |

---

## 🛠️ Desarrollo Local

### Compilar en Arduino IDE

```bash
# Abrir archivo
Arduino IDE → File → Open → firmware/zaque_device/zaque_device.ino

# Configurar placa
Tools → Board → ESP32 Dev Module
Tools → Port → /dev/ttyUSB0 (o COM3 en Windows)

# Compilar y subir
Sketch → Upload (Ctrl+U)
```

### Ver logs

```bash
# Monitor serial en Arduino IDE
Tools → Serial Monitor
Configurar: 115200 baud
```

### Debug

```cpp
// En config.h:
#define ENABLE_DEBUG_SERIAL true        // Habilita logs

// En código:
#if ENABLE_DEBUG_SERIAL
  Serial.println("✓ Mi mensaje de debug");
#endif
```

---

## 🎯 Próximas Fases

### 🔜 Phase 1: MAIN Autónomo
- [x] Lectura sensor Modbus
- [x] Lectura GPS
- [x] Almacenamiento SD
- [ ] Servidor web básico
- [ ] Dashboard HTML

### 🔜 Phase 2: API Local
- [ ] Endpoint POST /api/measurements
- [ ] Validación de datos
- [ ] Manejo de errores

### 🔜 Phase 3: Firmware SENSOR
- [ ] Lectura de sensores
- [ ] Envío HTTP al MAIN
- [ ] Deep sleep

### 🔜 Phase 4: Escalabilidad
- [ ] Soporte 3+ nodos
- [ ] Descubrimiento automático (mDNS)
- [ ] Sincronización de estado

### 🔜 Phase 5: Recomendaciones
- [ ] Motor de reglas completo
- [ ] Análisis predictivo
- [ ] Recomendaciones por cultivo

### 🔜 Phase 6-7: Futuro
- [ ] Sincronización con nube (opcional)
- [ ] IA para predicción
- [ ] Interfaz mejorada

---

## 📞 Preguntas Frecuentes

| Pregunta | Respuesta |
|---|---|
| **¿Necesito internet?** | No. Zaque funciona 100% offline con red WiFi local del celular. |
| **¿Cuánto cuesta?** | ~$100 USD por nodo (hardware accesible). |
| **¿Cuántos sensores?** | Teóricamente ilimitados. Límite práctico: rango WiFi (~50m). |
| **¿Si falla GPS?** | Se usa ubicación por defecto automáticamente, sin alertas. |
| **¿Se pierden datos sin internet?** | No. Todo se guarda localmente en microSD. |
| **¿Compatible con otros sensores?** | Sí, arquitectura modular permite integrar RS485/Modbus. |

---

## 📄 Licencia

```
ZAQUE - Sistema de Monitoreo Agrícola Local Distribuido
Copyright (C) 2026 Alejandro Roa, Laura Holguín, Andrés Guarnizo

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License.
```

Ver [LICENSE](LICENSE) para más detalles.

---

## 👥 Autores

- 🧑‍💻 **Alejandro Roa Aparicio** – Desarrollo firmware, arquitectura
- 👩‍💼 **Laura Sofía Holguín Giraldo** – Análisis agrícola, documentación
- 🔧 **Andrés Felipe Guarnizo Saavedra** – Hardware, integración sensores

---

<div align="center">

**🌾 Zaque: Empoderando a campesinos con tecnología accesible 🌾**

*Monitoreo inteligente, local, sin dependencia de internet.*

[📖 Documentación Completa](docs/) • [📱 Hardware](docs/hardware.md) • [🚀 Inicio Rápido](#-inicio-rápido) • [🤖 Recomendaciones](ZAQUE_RECOMMENDATION_ENGINE.md)

</div>
