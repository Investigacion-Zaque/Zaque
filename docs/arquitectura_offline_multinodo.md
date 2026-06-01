# Arquitectura - Zaque Offline Multinodo

## Visión General

Zaque es una red local de monitoreo agrícola distribuido que permite a los agricultores medir variables del suelo en múltiples puntos de su finca sin depender de internet.

```
┌──────────────────────────────────────────────────────────────┐
│              Red WiFi Local del Celular                      │
└──────────────────────────────────────────────────────────────┘
            ▲                ▲                ▲
            │                │                │
    ┌───────┴────────┐  ┌────┴─────────┐  ┌──┴──────────────┐
    │ ESP32 SENSOR 1 │  │ ESP32 SENSOR2│  │ ESP32 SENSOR N  │
    │ • RS485 sensor │  │ • RS485 sensor│  │ • RS485 sensor │
    │ • GPS          │  │ • GPS         │  │ • GPS          │
    │ • microSD      │  │ • microSD     │  │ • microSD      │
    │ • Batería      │  │ • Batería     │  │ • Batería      │
    └───┬────────────┘  └───┬──────────┘  └────┬───────────┘
        │ HTTP POST         │ HTTP POST         │ HTTP POST
        │                   │                   │
        └───────────────────┴───────────────────┘
                    ▼
        ┌──────────────────────────┐
        │   ESP32 MAIN             │
        │ • RS485 sensor (local)   │
        │ • GPS (local)            │
        │ • microSD (archivo único)│
        │ • Servidor web (HTTP)    │
        │ • API local              │
        │ • Dashboard              │
        │ • Recomendaciones        │
        └───┬────────────────────┬─┘
            │                    │
            │ HTTP GET           │ opcional: MQTT
            ▼                    ▼
    ┌────────────────┐   ┌──────────────┐
    │  Celular       │   │ Nube (futuro)│
    │ • Navegador    │   │ • Base datos │
    │ • Dashboard    │   │ • Análisis   │
    └────────────────┘   └──────────────┘
```

## Componentes

### 1. Nodo MAIN

**Rol**: Punto central de recolección, almacenamiento y visualización.

**Hardware**:
- ESP32 DEVKIT con conexión WiFi
- Sensor de suelo RS485/Modbus
- Módulo GPS
- Tarjeta microSD (32GB recomendado)
- Batería opcional (puede estar conectado 24/7)

**Responsabilidades**:
- Conectarse a red WiFi local del celular
- Leer sensor propio
- Leer GPS propio
- Levantar servidor web HTTP
- Exponer API local para sensores
- Recibir mediciones de sensores remotos
- Guardar todas las mediciones en SD
- Generar recomendaciones agrícolas
- Servir dashboard visual
- Manejar descarga de CSV/JSON

**Almacenamiento**:
```
/sd/
├── config.json              # Configuración del nodo
├── nodes.json               # Registry de nodos conocidos
├── latest.json              # Última medición de cada nodo
├── measurements.csv         # Histórico de mediciones
├── recommendations.csv      # Histó de recomendaciones
└── logs/
    └── system.log           # Eventos del sistema
```

### 2. Nodos SENSOR

**Rol**: Puntos distribuidos de medición.

**Hardware** (idéntico al MAIN):
- ESP32 DEVKIT con conexión WiFi
- Sensor de suelo RS485/Modbus
- Módulo GPS
- Tarjeta microSD
- Batería (generalmente esta sí)

**Responsabilidades**:
- Conectarse a red WiFi del celular
- Leer sensor local
- Leer GPS local
- Guardar medición local en SD
- Descubrir ubicación del MAIN (mDNS)
- Enviar medición al MAIN por HTTP POST
- Reintentar envío si falla
- Guardar en cola pendiente si no hay conexión
- Entrar en deep sleep para ahorrar batería
- Despertar según intervalo configurado

**Almacenamiento**:
```
/sd/
├── config.json              # Configuración local
├── local_measurements.csv   # Mediciones del nodo
├── pending_queue.csv        # Mediciones sin enviar
└── logs/
    └── sensor.log           # Eventos del sensor
```

## Comunicación

### Protocolo SENSOR → MAIN

**Método**: HTTP POST sobre WiFi local

**Endpoint**: `POST http://zaque.local/api/measurements`

**Payload JSON**:
```json
{
  "api_key": "ZAQUE_LOCAL_KEY",
  "node_id": "zona_2",
  "node_name": "Lote de maiz",
  "role": "sensor",
  "timestamp": "2026-05-31T16:20:00",
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

**Respuesta MAIN**:
```json
{
  "status": "ok",
  "stored": true,
  "recommendation": "Humedad adecuada y pH dentro de rango aceptable.",
  "server_time": "2026-05-31T16:20:05"
}
```

### API HTTP del MAIN

| Método | Ruta | Descripción |
|--------|------|-------------|
| GET | `/` | HTML dashboard |
| GET | `/dashboard` | Dashboard alternativo |
| GET | `/api/status` | Estado del sistema |
| GET | `/api/nodes` | Lista de nodos registrados |
| GET | `/api/measurements/latest` | Última medición de cada nodo |
| GET | `/api/history?node_id=zona_1` | Histórico de un nodo |
| POST | `/api/measurements` | Recibir medición de sensor |
| GET | `/download/measurements.csv` | Descargar CSV |

## Descubrimiento del MAIN

### Opción 1: mDNS (Preferida)
- El MAIN se anuncia como `zaque.local`
- Los sensores resuelven `http://zaque.local/api/measurements`
- Requiere soporte mDNS en red WiFi

### Opción 2: Fallback por IP
- Usar IP conocida: `192.168.43.50` (configurable)
- Solo funciona si el MAIN siempre recibe la misma IP

### Opción 3: UDP Discovery
- El MAIN responde broadcasts UDP en puerto específico
- Los sensores descubren MAIN automáticamente
- Implementación futura

## Flujos de Operación

### Inicio del Sistema

```
1. Campesino enciende zona móvil del celular
   └─ Crea red WiFi "NombreZonaMovil"

2. Enciende ESP32 MAIN
   └─ Conecta a WiFi
   └─ Inicia servidor web
   └─ Publica mDNS "zaque.local"
   └─ Espera mediciones

3. Enciende ESP32 SENSOR 1, SENSOR 2, ... SENSOR N
   └─ Cada uno conecta a WiFi
   └─ Cada uno busca "zaque.local"
   └─ Toman medición
   └─ Envían al MAIN
   └─ Guardan localmente
   └─ Entran en deep sleep
```

### Medición Periódica del SENSOR

```
Despertar por timer
  ↓
Conectar WiFi (retardo típico: 2-5s)
  ↓
Leer sensor local (retardo: variable)
  ↓
Leer GPS (retardo: 1-30s, depende de señal)
  ↓
Guardar en SD local
  ↓
Resolver zaque.local → IP
  ↓
POST a http://IP/api/measurements
  ├─ Si 200 OK: marcar como sincronizado
  └─ Si error: guardar en pending_queue.csv
  ↓
Deep sleep por N minutos (30 recomendado)
```

### Visualización en Dashboard

```
Campesino abre navegador en celular
  ↓
Entra a http://zaque.local
  ↓
Carga HTML dashboard (liviano, <100KB)
  ↓
JavaScript hace:
  └─ GET /api/measurements/latest
  └─ Renderiza tarjetas por nodo
  └─ Muestra coordenadas GPS
  └─ Muestra recomendaciones
  └─ Muestra batería
  ↓
Campesino consulta datos de su finca
```

## Almacenamiento de Datos

### Formato CSV Global (MAIN)

Archivo: `/measurements.csv`

```csv
timestamp,node_id,node_name,lat,lon,soil_temperature,soil_humidity,electrical_conductivity,ph,nitrogen,phosphorus,potassium,battery_percent,recommendation
2026-05-31T16:20:00,main,Nodo principal,4.7110,-74.0710,22.5,58,750,6.4,35,20,40,91,"Condiciones estables"
2026-05-31T16:20:05,zona_2,Lote de maiz,4.7112,-74.0721,22.8,64,820,6.2,34,18,41,83,"Humedad adecuada"
2026-05-31T16:20:10,zona_3,Cafetal,4.7120,-74.0730,21.2,52,600,5.8,28,16,32,71,"pH ligeramente ácido"
```

### Archivo latest.json (MAIN)

Actualizado después de cada medición. Permite que el dashboard cargue rápido.

```json
{
  "updated_at": "2026-05-31T16:20:10",
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
      "last_seen": "2026-05-31T16:20:00",
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
      "last_seen": "2026-05-31T16:20:05",
      "recommendation": "Humedad adecuada."
    }
  ]
}
```

## Recomendaciones Agrícolas

Motor de reglas simple basado en umbrales:

```
SI humedad < 30% → "Revisar riego"
SI humedad > 80% → "Verificar drenaje"
SI pH < 5.5     → "Suelo ácido, considerar enmienda"
SI pH > 7.5     → "Suelo alcalino, revisar tipo de fertilizante"
SI EC > 2000    → "Posible exceso de sales"
SI NPK < 30     → "Revisar fertilización"
SI batería < 20% → "Cargar nodo"
```

Las recomendaciones son orientativas y se muestran con disclaimer.

## Bajo Consumo (SENSOR)

Para maximizar autonomía de batería:

1. **WiFi**: Solo conectar cuando sea necesario (no mantener conexión)
2. **Lectura**: Tomar medición en ~5-10 segundos
3. **Transmisión**: Enviar datos en ~2-5 segundos
4. **Deep Sleep**: El 99% del tiempo en dormición

Con intervalo de 30 minutos:
- Tiempo activo: ~15 segundos
- Tiempo durmiendo: ~1799 segundos
- Consumo promedio: muy bajo

Puede funcionar de 1-3 meses con batería adecuada.

## Seguridad (MVP)

- API key simple en header (futuro: JWT)
- HTTPS no (futuro: cuando haya conectividad)
- Validación de JSON básica
- Sin autenticación (futuro)

## Escalabilidad

- Teórico: ESP32 puede servir múltiples clientes
- Práctico: Probar con 3-5 nodos
- Limitante: Hotspot del celular

Si necesita >10 nodos, usar router WiFi adicional.
