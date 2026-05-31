# Informe técnico: Migración de Zaque hacia una red local multinodo con dashboard offline

## 1. Resumen ejecutivo

El proyecto **Zaque** actualmente está planteado como un sistema de monitoreo ambiental de suelos para comunidades campesinas de Colombia. La arquitectura actual usa nodos ESP32 que miden variables del suelo y envían datos mediante **ESP-NOW** hacia un receptor/gateway, el cual publica la información por **MQTT** hacia un servidor Node.js y una base de datos MySQL.

La nueva versión propuesta cambia el enfoque hacia una arquitectura **local, autónoma y tolerante a falta de internet**, donde un campesino pueda encender la zona móvil de su celular, conectar varios dispositivos ESP32 a esa red local y visualizar las mediciones de diferentes puntos de la finca desde una página web levantada por un nodo principal.

La propuesta consiste en migrar el proyecto hacia una arquitectura con:

- Un **nodo MAIN**, que funciona como estación principal.
- Varios **nodos SENSOR**, que funcionan como puntos distribuidos de medición.
- Mismo hardware para todos los dispositivos.
- Diferencia únicamente por firmware o configuración.
- Comunicación por WiFi local usando la red del celular.
- Dashboard web servido desde el nodo MAIN.
- Almacenamiento local en microSD.
- GPS en cada nodo para georreferenciar las mediciones.
- Recomendaciones agrícolas básicas por zona.
- MQTT y nube como modo opcional futuro, no como dependencia principal.

---

## 2. Estado actual del repositorio

Según la estructura actual del repositorio, Zaque contiene los siguientes módulos principales:

```text
Zaque/
├── ESP32/
├── Base de datos/
├── Api/
├── Frontend/
├── Backend/
├── README.md
└── LICENSE
```

El README actual describe una arquitectura basada en:

```text
[ESP32 Emisor] ──ESP-NOW──> [ESP32 Receptor] ──MQTT──> [Servidor Node.js] ──> [MySQL]
```

Los componentes actuales relevantes son:

| Componente actual                       |    Estado | Uso actual                                                                  |
| --------------------------------------- | --------: | --------------------------------------------------------------------------- |
| `ESP32/ESPNOW_bajo_consumo_emisor.ino`  | Funcional | Nodo sensor que lee RS485/Modbus, envía por ESP-NOW y entra en deep sleep. |
| `ESP32/Receptor_con_MQTT_y_Bateria.ino` | Funcional | Nodo receptor/gateway que recibe ESP-NOW y publica por MQTT.                |
| `Base de datos/server.js`               | Funcional | Servidor Node.js con cliente MQTT y persistencia en MySQL.                  |
| `Api/`                                  | Pendiente | REST API futura.                                                            |
| `Frontend/`                             | Pendiente | Dashboard futuro.                                                           |
| `Backend/`                              | Pendiente | Lógica de negocio futura.                                                  |

La versión actual ya tiene una base valiosa:

- Lectura de sensor NPK por RS485/Modbus.
- Comunicación entre ESP32.
- Medición de batería.
- Deep sleep para bajo consumo.
- Separación entre nodo sensor y nodo receptor.
- Backend inicial para recibir datos por MQTT.

Sin embargo, el nuevo objetivo requiere reducir la dependencia de internet y de infraestructura externa.

---

## 3. Nuevo objetivo del proyecto

El nuevo objetivo de Zaque es convertirse en una **red local de monitoreo agrícola distribuido**, donde un campesino pueda consultar información del suelo de diferentes zonas de su finca sin depender de internet.

La nueva idea central es:

> Un conjunto de dispositivos ESP32 con el mismo hardware, distribuidos en distintos puntos de una finca. Uno de ellos se configura como nodo MAIN y levanta un dashboard web local. Los demás se configuran como nodos SENSOR y envían sus mediciones al MAIN mediante la red WiFi creada por el celular del campesino. Cada nodo guarda sus datos localmente en microSD y reporta mediciones georreferenciadas con GPS.

---

## 4. Arquitectura propuesta

### 4.1 Arquitectura general

```text
Red WiFi local del celular
                                  │
                                  │
        ┌─────────────────────────┼─────────────────────────┐
        │                         │                         │
        ▼                         ▼                         ▼
┌─────────────────┐       ┌─────────────────┐       ┌─────────────────┐
│ ESP32 SENSOR 1  │       │ ESP32 SENSOR 2  │       │ ESP32 SENSOR N  │
│ Sensor suelo    │       │ Sensor suelo    │       │ Sensor suelo    │
│ GPS             │       │ GPS             │       │ GPS             │
│ microSD         │       │ microSD         │       │ microSD         │
│ Batería         │       │ Batería         │       │ Batería         │
└────────┬────────┘       └────────┬────────┘       └────────┬────────┘
         │                         │                         │
         │ HTTP/JSON               │ HTTP/JSON               │ HTTP/JSON
         └─────────────────────────┼─────────────────────────┘
                                   ▼
                         ┌─────────────────┐
                         │ ESP32 MAIN      │
                         │ Sensor suelo    │
                         │ GPS             │
                         │ microSD         │
                         │ Web dashboard   │
                         │ API local       │
                         │ Recomendaciones │
                         └────────┬────────┘
                                  │
                                  │ HTTP local
                                  ▼
                         ┌─────────────────┐
                         │ Celular         │
                         │ Navegador web   │
                         │ Dashboard Zaque │
                         └─────────────────┘
```

### 4.2 Roles del sistema

#### Nodo MAIN

El nodo MAIN es un dispositivo completo. No es solo un gateway; también mide su propia zona.

Funciones:

- Conectarse al WiFi del celular.
- Leer su propio sensor de suelo.
- Leer GPS.
- Guardar sus propias mediciones en microSD.
- Levantar un servidor web local.
- Exponer una API HTTP para recibir datos de otros nodos.
- Guardar las mediciones recibidas de los nodos SENSOR.
- Calcular o asignar recomendaciones agrícolas básicas.
- Mostrar dashboard con todos los puntos de medición.
- Permitir descarga de archivos CSV o JSON.

#### Nodo SENSOR

El nodo SENSOR tiene el mismo hardware que el MAIN, pero ejecuta un firmware distinto o una configuración diferente.

Funciones:

- Conectarse al WiFi del celular.
- Leer sensor de suelo.
- Leer GPS.
- Guardar copia local en microSD.
- Buscar o conocer la IP del nodo MAIN.
- Enviar mediciones por HTTP/JSON al nodo MAIN.
- Reintentar envío si falla.
- Dormir entre mediciones para ahorrar batería.

---

## 5. Decisión clave: mismo hardware, firmware diferente

Una de las mejores decisiones del nuevo diseño es que todos los dispositivos compartan el mismo hardware:

```text
ESP32 + sensor de suelo RS485/Modbus + GPS + microSD + batería
```

La diferencia se define por software:

```cpp
#define DEVICE_ROLE_MAIN 1
#define DEVICE_ROLE_SENSOR 2

#define DEVICE_ROLE DEVICE_ROLE_MAIN
// #define DEVICE_ROLE DEVICE_ROLE_SENSOR
```

Ventajas:

- Simplifica compras y ensamblaje.
- Facilita mantenimiento.
- Permite reemplazar un MAIN por un SENSOR cambiando firmware.
- Permite escalar de 1 nodo a muchos nodos.
- Reduce errores de cableado por diseños diferentes.
- Facilita documentar un solo diagrama de hardware.

---

## 6. Comparación entre arquitectura actual y nueva arquitectura

| Aspecto                   | Arquitectura actual          | Nueva arquitectura propuesta       |
| ------------------------- | ---------------------------- | ---------------------------------- |
| Comunicación entre nodos | ESP-NOW                      | WiFi local del celular + HTTP/JSON |
| Salida principal de datos | MQTT hacia servidor          | Dashboard web local en el MAIN     |
| Dependencia de internet   | Alta                         | Baja o nula                        |
| Almacenamiento            | MySQL en servidor            | microSD local en MAIN y sensores   |
| Visualización            | Frontend pendiente           | Web embebida en ESP32 MAIN         |
| Escalabilidad local       | Emisor + receptor            | 1 MAIN + muchos SENSOR             |
| GPS                       | No central en diseño actual | GPS por nodo                       |
| Recomendaciones           | Backend futuro               | Reglas locales por zona            |
| Uso en campo sin internet | Limitado                     | Principal objetivo                 |
| Nube/MQTT                 | Principal                    | Opcional futuro                    |

---

## 7. Flujo de operación propuesto

### 7.1 Flujo para el campesino

```text
1. El campesino prende la zona móvil del celular.
2. Enciende el nodo MAIN.
3. Enciende los nodos SENSOR distribuidos en la finca.
4. Todos los ESP32 se conectan a la red WiFi local del celular.
5. El MAIN levanta el dashboard web.
6. Los SENSOR toman mediciones y las envían al MAIN.
7. El campesino abre el navegador del celular.
8. Entra a la IP del MAIN o a zaque.local si mDNS funciona.
9. Visualiza mediciones, coordenadas, batería y recomendaciones por zona.
```

### 7.2 Flujo técnico del nodo MAIN

```text
Inicio
  ↓
Cargar configuración
  ↓
Inicializar sensor RS485/Modbus
  ↓
Inicializar GPS
  ↓
Inicializar microSD
  ↓
Conectarse al WiFi del celular
  ↓
Levantar servidor web
  ↓
Levantar API local
  ↓
Tomar medición propia
  ↓
Guardar medición propia en SD
  ↓
Recibir mediciones de sensores
  ↓
Guardar mediciones recibidas
  ↓
Actualizar archivo resumen
  ↓
Mostrar dashboard
```

### 7.3 Flujo técnico del nodo SENSOR

```text
Inicio
  ↓
Cargar configuración del nodo
  ↓
Inicializar sensor RS485/Modbus
  ↓
Inicializar GPS
  ↓
Inicializar microSD
  ↓
Conectarse al WiFi del celular
  ↓
Encontrar MAIN
  ↓
Tomar medición
  ↓
Guardar medición local en SD
  ↓
Enviar medición al MAIN por HTTP POST
  ↓
Si envío correcto: marcar como sincronizado
  ↓
Si envío falla: dejar pendiente para reintento
  ↓
Entrar en deep sleep
```

---

## 8. Comunicación entre SENSOR y MAIN

### 8.1 Protocolo recomendado

Para esta nueva versión, se recomienda usar:

```text
WiFi local + HTTP + JSON
```

Motivos:

- Es fácil de implementar.
- Es fácil de depurar con navegador, Postman o curl.
- Se integra naturalmente con el servidor web del MAIN.
- Permite que los sensores usen una API clara.
- Facilita migrar después a una API externa o sincronización en nube.

### 8.2 Rutas HTTP sugeridas en el MAIN

```text
GET  /
GET  /dashboard
GET  /api/status
GET  /api/nodes
GET  /api/measurements/latest
GET  /api/history?node_id=zona_1
POST /api/measurements
GET  /download/measurements.csv
```

### 8.3 Payload JSON enviado por cada SENSOR

```json
{
  "node_id": "zona_2",
  "node_name": "Lote de maiz",
  "role": "sensor",
  "timestamp": "2026-05-30T16:20:00",
  "lat": 4.7112,
  "lon": -74.0721,
  "soil_temperature": 22.8,
  "soil_humidity": 64.0,
  "electrical_conductivity": 820,
  "ph": 6.2,
  "nitrogen": 34,
  "phosphorus": 18,
  "potassium": 41,
  "battery_percent": 83,
  "firmware_version": "0.2.0"
}
```

### 8.4 Respuesta del MAIN

```json
{
  "status": "ok",
  "stored": true,
  "recommendation": "Humedad adecuada y pH dentro de rango aceptable.",
  "server_time": "2026-05-30T16:20:05"
}
```

---

## 9. Descubrimiento del nodo MAIN

Uno de los retos principales es que los nodos SENSOR necesitan saber la dirección IP del MAIN.

Se proponen implementación.

### mDNS

El MAIN se anuncia como:

```text
zaque.local
```

Los sensores intentan enviar a:

```text
http://zaque.local/api/measurements
```

Ventaja:

- Más amigable.
- No requiere conocer IP.

Desventaja:

- Puede fallar en algunos celulares o redes.

## 10. Almacenamiento en microSD

### 10.1 Por qué usar SD en todos los nodos

Aunque el MAIN guarde todos los datos, conviene que cada SENSOR también tenga microSD.

Ventajas:

- Si no hay conexión con el MAIN, el SENSOR no pierde datos.
- Cada nodo tiene respaldo local.
- Se puede recuperar información directamente desde la SD.
- Permite sincronización posterior.
- Mejora la confiabilidad en campo.

### 10.2 Archivos recomendados en el MAIN

```text
/sd/
├── config.json
├── nodes.json
├── latest.json
├── measurements.csv
├── recommendations.csv
└── logs/
    └── system.log
```

### 10.3 Archivos recomendados en cada SENSOR

```text
/sd/
├── config.json
├── local_measurements.csv
├── pending_queue.csv
└── logs/
    └── sensor.log
```

### 10.4 Formato CSV global del MAIN

```csv
timestamp,node_id,node_name,lat,lon,soil_temperature,soil_humidity,electrical_conductivity,ph,nitrogen,phosphorus,potassium,battery_percent,recommendation
2026-05-30T16:20:00,zona_2,Lote de maiz,4.7112,-74.0721,22.8,64,820,6.2,34,18,41,83,"Humedad adecuada y pH aceptable"
```

### 10.5 Archivo `latest.json`

Este archivo facilita que el dashboard cargue rápidamente la última medición de cada nodo.

```json
{
  "updated_at": "2026-05-30T16:20:05",
  "nodes": [
    {
      "node_id": "main",
      "node_name": "Nodo principal",
      "role": "main",
      "lat": 4.7110,
      "lon": -74.0710,
      "soil_humidity": 58,
      "ph": 6.4,
      "nitrogen": 35,
      "phosphorus": 20,
      "potassium": 40,
      "battery_percent": 91,
      "last_seen": "2026-05-30T16:19:00",
      "recommendation": "Condiciones estables."
    },
    {
      "node_id": "zona_2",
      "node_name": "Lote de maiz",
      "role": "sensor",
      "lat": 4.7112,
      "lon": -74.0721,
      "soil_humidity": 64,
      "ph": 6.2,
      "nitrogen": 34,
      "phosphorus": 18,
      "potassium": 41,
      "battery_percent": 83,
      "last_seen": "2026-05-30T16:20:00",
      "recommendation": "Humedad adecuada y pH aceptable."
    }
  ]
}
```

---

## 11. Recomendaciones agrícolas locales

La primera versión no debería intentar usar inteligencia artificial compleja. Se recomienda comenzar con un motor de reglas simple, explicable y fácil de ajustar.

### 11.1 Variables consideradas

- Humedad del suelo.
- pH.
- Conductividad eléctrica.
- Nitrógeno.
- Fósforo.
- Potasio.
- Temperatura del suelo.
- Batería del nodo.

### 11.2 Ejemplo de reglas iniciales

```text
Si humedad < umbral_bajo:
  Recomendar revisar riego.

Si pH < 5.5:
  Indicar posible acidez del suelo.

Si pH > 7.5:
  Indicar posible alcalinidad del suelo.

Si conductividad es muy alta:
  Alertar posible exceso de sales.

Si N, P o K están bajos:
  Recomendar revisar fertilización según cultivo.

Si batería < 20%:
  Recomendar recargar o revisar alimentación del nodo.
```

### 11.3 Recomendación importante

Las recomendaciones deben mostrarse como orientación, no como diagnóstico absoluto.

Texto sugerido para el dashboard:

> Las recomendaciones son orientativas y deben ajustarse según el cultivo, etapa de crecimiento, tipo de suelo y acompañamiento técnico local.

---

## 12. Dashboard web local

### 12.1 Objetivo del dashboard

El dashboard debe ser simple, liviano y funcional en celulares.

Debe permitir:

- Ver todos los nodos conectados.
- Ver última medición por zona.
- Ver coordenadas GPS.
- Ver batería de cada nodo.
- Ver recomendaciones por zona.
- Ver histórico básico.
- Descargar CSV.
- Identificar nodos sin conexión reciente.

### 12.2 Vista principal sugerida

```text
Zaque - Monitoreo local de finca

Resumen:
- Nodos registrados: 4
- Nodos activos: 3
- Alertas: 2
- Última actualización: hace 1 minuto

[MAIN - Casa finca]
Humedad: 58%
PH: 6.4
NPK: 35 / 20 / 40
Batería: 91%
Recomendación: Condiciones estables.

[Zona 2 - Lote de maíz]
Humedad: 34%
PH: 5.3
NPK: 18 / 10 / 24
Batería: 76%
Recomendación: Humedad baja y pH ácido. Revisar riego y corrección del suelo.
```

### 12.3 Mapa offline

Como no se puede depender de Google Maps o mapas online si no hay internet, se recomienda iniciar con dos opciones:

#### Opción básica

Mostrar coordenadas GPS en texto:

```text
Zona 1: 4.7112, -74.0721
Zona 2: 4.7120, -74.0730
```

#### Opción intermedia

Crear un mapa relativo de finca usando las diferencias entre coordenadas:

```text
Zona 2
           ●

Zona 1 ●          ● Zona 3

           ●
         MAIN
```

Este mapa no requiere internet y puede renderizarse con HTML, CSS y JavaScript sencillo.

---

## 13. Nueva estructura recomendada del repositorio

Se recomienda no borrar de inmediato la arquitectura anterior. Lo mejor es migrar progresivamente y dejar el modo MQTT como referencia o modo online futuro.

```text
Zaque/
├── firmware/
│   ├── zaque_device/
│   │   ├── zaque_device.ino
│   │   ├── config.h
│   │   ├── roles.h
│   │   ├── sensor_reader.cpp
│   │   ├── sensor_reader.h
│   │   ├── gps_reader.cpp
│   │   ├── gps_reader.h
│   │   ├── sd_logger.cpp
│   │   ├── sd_logger.h
│   │   ├── wifi_manager.cpp
│   │   ├── wifi_manager.h
│   │   ├── http_client.cpp
│   │   ├── http_client.h
│   │   ├── web_server.cpp
│   │   ├── web_server.h
│   │   ├── recommendations.cpp
│   │   ├── recommendations.h
│   │   ├── node_registry.cpp
│   │   ├── node_registry.h
│   │   └── dashboard_html.h
│   │
│   └── legacy/
│       ├── ESPNOW_bajo_consumo_emisor.ino
│       └── Receptor_con_MQTT_y_Bateria.ino
│
├── docs/
│   ├── arquitectura_offline_multinodo.md
│   ├── hardware.md
│   ├── pinout.md
│   ├── protocolo_api_local.md
│   ├── formato_sd.md
│   ├── recomendaciones.md
│   └── plan_migracion.md
│
├── dashboard/
│   ├── prototype.html
│   ├── styles.css
│   └── app.js
│
├── tools/
│   ├── csv_to_json.py
│   └── sd_validator.py
│
├── cloud_optional/
│   ├── mqtt_bridge/
│   ├── api/
│   └── database/
│
├── README.md
└── LICENSE
```

### 13.1 Qué hacer con las carpetas actuales

| Carpeta actual   | Acción recomendada                                                               |
| ---------------- | --------------------------------------------------------------------------------- |
| `ESP32/`         | Migrar a`firmware/legacy/` y crear `firmware/zaque_device/`.                      |
| `Base de datos/` | Mover a`cloud_optional/mqtt_bridge/` o mantener como modo online.                 |
| `Api/`           | Usar para futura API externa, no para MVP offline.                                |
| `Frontend/`      | Usar como prototipo visual; la versión embebida debe vivir en`dashboard_html.h`. |
| `Backend/`       | Reservar para lógica futura de nube o sincronización.                           |
| `README.md`      | Actualizar con la nueva arquitectura offline multinodo.                           |

---

## 14. Firmware unificado por roles

### 14.1 Archivo `config.h`

```cpp
#pragma once

#define DEVICE_ROLE_MAIN 1
#define DEVICE_ROLE_SENSOR 2

#define DEVICE_ROLE DEVICE_ROLE_MAIN

#define NODE_ID "main"
#define NODE_NAME "Nodo principal - Casa finca"
#define FIRMWARE_VERSION "0.2.0"

#define WIFI_SSID "NombreZonaMovil"
#define WIFI_PASSWORD "ClaveZonaMovil"

#define MAIN_HOST "zaque.local"
#define MAIN_IP_FALLBACK "192.168.43.50"
#define MAIN_PORT 80

#define MEASUREMENT_INTERVAL_MINUTES 30
```

Para un sensor:

```cpp
#define DEVICE_ROLE DEVICE_ROLE_SENSOR
#define NODE_ID "zona_2"
#define NODE_NAME "Lote de maiz"
```

### 14.2 Pseudocódigo principal

```cpp
#include "config.h"
#include "sensor_reader.h"
#include "gps_reader.h"
#include "sd_logger.h"
#include "wifi_manager.h"
#include "recommendations.h"

#if DEVICE_ROLE == DEVICE_ROLE_MAIN
#include "web_server.h"
#include "node_registry.h"
#endif

#if DEVICE_ROLE == DEVICE_ROLE_SENSOR
#include "http_client.h"
#endif

void setup() {
  initSerial();
  initSensor();
  initGPS();
  initSD();
  connectWiFi();

#if DEVICE_ROLE == DEVICE_ROLE_MAIN
  initNodeRegistry();
  startWebServer();
#endif
}

void loop() {
  Measurement m = readMeasurement();
  m.recommendation = buildRecommendation(m);
  saveMeasurementToSD(m);

#if DEVICE_ROLE == DEVICE_ROLE_MAIN
  updateLocalNode(m);
  handleWebServer();
#endif

#if DEVICE_ROLE == DEVICE_ROLE_SENSOR
  bool sent = sendMeasurementToMain(m);
  if (!sent) {
    savePendingMeasurement(m);
  }
  goToDeepSleep(MEASUREMENT_INTERVAL_MINUTES);
#endif
}
```

---

## 15. Migración del código actual

### 15.1 Reutilizar del emisor actual

Del archivo actual del emisor ESP-NOW se puede reutilizar:

- Inicialización del sensor RS485/Modbus.
- Lectura de variables del suelo.
- Cálculo de batería.
- Estructura de medición.
- Lógica de deep sleep.

Cambios necesarios:

- Reemplazar envío ESP-NOW por HTTP POST.
- Agregar GPS.
- Agregar microSD.
- Agregar reintento de envío.
- Agregar cola de datos pendientes.

### 15.2 Reutilizar del receptor actual

Del archivo actual del receptor con MQTT se puede reutilizar:

- Idea de gateway central.
- Manejo de WiFi.
- Manejo de datos recibidos.
- Estructura de publicación de datos.

Cambios necesarios:

- Reemplazar publicación MQTT por almacenamiento en SD.
- Agregar servidor web embebido.
- Agregar API HTTP local.
- Agregar lectura del sensor propio del MAIN.
- Agregar GPS propio.
- Agregar dashboard.

### 15.3 Reutilizar del backend Node.js

El servidor Node.js actual no debe ser eliminado. Se recomienda moverlo a una carpeta de modo online opcional.

Uso futuro:

- Sincronización cuando haya internet.
- Carga de CSV históricos.
- Panel web en nube.
- Respaldo de datos.
- Análisis avanzado.

---

## 16. Plan de migración por fases


### Fase 1: MAIN autónomo de un solo nodo

Objetivo:

```text
Un solo ESP32 mide, guarda en SD y levanta web local.
```

Tareas:

- Integrar lectura del sensor actual.
- Agregar GPS.
- Agregar microSD.
- Crear archivo CSV local.
- Levantar servidor web simple.
- Mostrar última medición.

Resultado esperado:

```text
ESP32 MAIN → Dashboard local con su propia medición
```

### Fase 2: API local del MAIN

Objetivo:

```text
Permitir que otros nodos envíen mediciones al MAIN.
```

Tareas:

- Crear endpoint `POST /api/measurements`.
- Validar JSON recibido.
- Guardar medición en `measurements.csv`.
- Actualizar `latest.json`.
- Responder con recomendación.

Resultado esperado:

```text
curl o Postman puede enviar una medición falsa al MAIN y aparece en el dashboard.
```

### Fase 3: Firmware SENSOR

Objetivo:

```text
Un nodo SENSOR mide y envía datos reales al MAIN.
```

Tareas:

- Configurar firmware como SENSOR.
- Conectar al WiFi del celular.
- Leer sensor y GPS.
- Guardar localmente en SD.
- Enviar JSON al MAIN.
- Implementar reintento si falla.
- Implementar deep sleep.

Resultado esperado:

```text
SENSOR → HTTP POST → MAIN → SD → Dashboard
```

### Fase 4: Múltiples sensores

Objetivo:

```text
Escalar a 3 o más puntos de medición.
```

Tareas:

- Crear IDs únicos por nodo.
- Mostrar tarjetas por nodo.
- Detectar último contacto.
- Marcar nodos inactivos.
- Mostrar batería por nodo.

Resultado esperado:

```text
Dashboard con MAIN + SENSOR 1 + SENSOR 2 + SENSOR 3
```

### Fase 5: Recomendaciones por zona

Objetivo:

```text
Mostrar alertas y recomendaciones por nodo.
```

Tareas:

- Crear `recommendations.cpp`.
- Definir reglas base.
- Asociar recomendaciones a cada medición.
- Mostrar nivel de alerta.

Resultado esperado:

```text
Cada zona muestra estado: estable, atención, crítico.
```

### Fase 6: Descubrimiento automático del MAIN

Objetivo:

```text
Evitar depender de IP fija.
```

Tareas:

- Implementar mDNS `zaque.local`.
- Implementar fallback por IP.
- Implementar UDP discovery si mDNS falla.

Resultado esperado:

```text
Los sensores encuentran al MAIN automáticamente.
```

### Fase 7: Sincronización opcional con nube

Objetivo:

```text
Recuperar MQTT/API solo cuando haya internet.
```

Tareas:

- Crear módulo opcional `cloud_sync`.
- Subir CSV o JSON a servidor.
- Reusar servidor Node.js actual.
- Mantener modo offline como prioridad.

Resultado esperado:

```text
Zaque funciona sin internet, pero puede sincronizar cuando haya conexión.
```

---

## 17. Librerías sugeridas para ESP32

### Para firmware Arduino

```text
ModbusMaster       → sensor RS485/Modbus
TinyGPSPlus        → lectura GPS
SD                 → tarjeta microSD
SPI                → comunicación con SD
WiFi               → conexión al hotspot
WebServer          → servidor HTTP simple
HTTPClient         → envío HTTP desde SENSOR a MAIN
ArduinoJson        → manejo de JSON
ESPmDNS            → zaque.local
```

### Notas

- Si el dashboard crece mucho, considerar `ESPAsyncWebServer`, pero para iniciar basta con `WebServer`.
- `ArduinoJson` debe usarse con documentos pequeños para no saturar memoria.
- El HTML del dashboard debe ser liviano.

---

## 18. Consideraciones de hardware

### 18.1 Hardware base por nodo

Cada nodo tendría:

```text
- ESP32
- Sensor de suelo RS485/Modbus NPK
- Módulo RS485 tipo HW-519 o equivalente
- Módulo GPS
- Módulo microSD
- Batería
- Regulador de voltaje adecuado
- Caja protectora
```

### 18.2 Recomendación de pines

Se debe diseñar un pinout definitivo evitando conflictos entre:

- UART del sensor RS485.
- UART del GPS.
- SPI de la microSD.
- ADC para batería.
- Pines de boot del ESP32.

Ejemplo conceptual:

```text
RS485/Modbus → UART2
GPS          → UART1
microSD      → SPI
Batería      → ADC
```

### 18.3 Riesgos de hardware

- El GPS puede consumir bastante energía.
- La SD puede corromper datos si se apaga durante escritura.
- El sensor RS485 puede requerir alimentación diferente al ESP32.
- El WiFi consume más que ESP-NOW.
- El hotspot del celular puede limitar el número de dispositivos conectados.

---

## 19. Energía y bajo consumo

Para nodos SENSOR se recomienda:

```text
Despertar → conectar WiFi → medir → guardar SD → enviar → dormir
```

Intervalos sugeridos:

```text
15 minutos: monitoreo más frecuente
30 minutos: equilibrio entre datos y batería
60 minutos: mayor autonomía
```

El MAIN probablemente debe permanecer activo mientras el campesino consulta el dashboard. Se puede considerar un modo donde el MAIN también duerma cuando no se necesita la web, pero para el MVP debe permanecer encendido.

---

## 20. Seguridad mínima

Aunque el sistema sea local, conviene implementar seguridad básica.

Recomendaciones:

- No dejar contraseñas reales en GitHub.
- Mantener `secrets.h` fuera del repositorio.
- Usar `secrets_example.h` como plantilla.
- Agregar un token simple para que solo nodos autorizados envíen datos.

Ejemplo:

```json
{
  "api_key": "ZAQUE_LOCAL_KEY",
  "node_id": "zona_2",
  "soil_humidity": 64
}
```

El MAIN rechaza mediciones sin `api_key` válida.

---

## 21. Cambios sugeridos al README

El README debería actualizarse para explicar la nueva visión:

```markdown
# Zaque

Zaque es una red local de monitoreo agrícola distribuido para comunidades campesinas. El sistema permite instalar varios nodos ESP32 en diferentes puntos de una finca para medir variables del suelo, registrar coordenadas GPS, almacenar datos en microSD y visualizar un dashboard web local desde el celular del agricultor, incluso sin conexión a internet.
```

Nueva arquitectura en README:

```text
[SENSOR 1] ─┐
[SENSOR 2] ─┼── WiFi local / HTTP ──> [MAIN + Web + SD] ──> [Celular]
[SENSOR N] ─┘
```

Se debe aclarar que MQTT pasa a ser opcional:

```markdown
La versión anterior basada en MQTT se conserva como modo online opcional para sincronización futura con servidor y base de datos.
```

---

## 22. Backlog técnico recomendado

### Prioridad alta

- Crear firmware unificado con roles MAIN/SENSOR.
- Migrar lectura RS485/Modbus del firmware actual.
- Implementar SD logger.
- Implementar dashboard básico.
- Implementar `POST /api/measurements`.
- Implementar envío HTTP desde SENSOR.
- Definir formato JSON y CSV.

### Prioridad media

- Agregar GPS.
- Agregar recomendaciones por reglas.
- Agregar vista por nodos.
- Agregar descarga CSV.
- Agregar detección de nodos inactivos.
- Agregar mDNS.

### Prioridad baja

- UDP discovery.
- Mapa relativo offline.
- Sincronización con nube.
- Panel avanzado con gráficas.
- Carga automática de datos históricos.

---

## 23. Riesgos del nuevo enfoque

| Riesgo                                    | Impacto | Mitigación                                                 |
| ----------------------------------------- | ------: | ----------------------------------------------------------- |
| El celular limita dispositivos conectados |   Medio | Probar con 3-5 nodos; para más nodos usar router local.    |
| IP del MAIN cambia                        |    Alto | mDNS, IP fija o UDP discovery.                              |
| Consumo alto por WiFi/GPS                 |    Alto | Deep sleep en sensores.                                     |
| Corrupción de SD                         |   Medio | Escribir de forma controlada y cerrar archivos.             |
| Dashboard pesado                          |   Medio | HTML/CSS/JS liviano.                                        |
| GPS sin señal                            |   Medio | Permitir coordenadas manuales o última coordenada válida. |
| Pérdida de conexión con MAIN            |   Medio | Guardar cola pendiente en SD.                               |
| Memoria limitada ESP32                    |   Medio | JSON pequeño y archivos estáticos simples.                |

---

## 24. MVP recomendado

El MVP más realista sería:

```text
1 nodo MAIN
1 nodo SENSOR
Ambos con mismo hardware
Red WiFi creada por celular
MAIN levanta dashboard
SENSOR envía datos por HTTP
MAIN guarda CSV en SD
Dashboard muestra ambos nodos
```

Criterios de éxito:

- El MAIN se conecta al hotspot.
- El dashboard abre desde el celular.
- El MAIN muestra su propia medición.
- El SENSOR envía una medición real.
- El MAIN guarda la medición en SD.
- La web muestra la medición del SENSOR.
- Se puede descargar o consultar el CSV.
- Si el SENSOR no logra enviar, guarda el dato localmente.

---

## 25. Conclusión

La migración propuesta transforma Zaque de una arquitectura dependiente de MQTT, servidor y base de datos en la nube hacia una arquitectura local, autónoma y más adecuada para zonas rurales.

La nueva versión permite que un campesino use únicamente la zona móvil del celular como red local, sin necesidad de internet, para consultar las condiciones del suelo en diferentes puntos de su finca.

La decisión de usar el mismo hardware para todos los nodos y diferenciar únicamente el firmware es una decisión técnica fuerte porque reduce complejidad, facilita mantenimiento y permite escalar el sistema de forma ordenada.

La arquitectura recomendada para la nueva versión es:

```text
Mismo hardware para todos los nodos
+ Firmware MAIN para estación principal
+ Firmware SENSOR para puntos distribuidos
+ WiFi local del celular
+ API HTTP local
+ microSD en todos los nodos
+ dashboard web embebido
+ recomendaciones por reglas
+ MQTT opcional futuro
```

Esta migración conserva lo valioso del proyecto actual, especialmente la lectura de sensores y la lógica de bajo consumo, pero orienta el sistema hacia una solución más útil, demostrable y realista para el contexto rural colombiano.

---

## 26. Referencias

- Repositorio actual de Zaque: https://github.com/Investigacion-Zaque/Zaque/tree/main
- Arquitectura actual documentada en el README del repositorio: ESP32 emisor por ESP-NOW, ESP32 receptor con MQTT, servidor Node.js y MySQL.

