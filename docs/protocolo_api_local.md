# Protocolo API Local - Zaque Offline

## Overview

El API local es una interfaz HTTP simple que permite a los nodos SENSOR enviar mediciones y obtener recomendaciones del nodo MAIN.

El objetivo es ser simple, liviano y sin dependencias externas de autenticación complicada.

## Endpoints del MAIN

### GET /
Retorna el dashboard HTML.

**Respuesta**:
- Content-Type: `text/html`
- Body: HTML del dashboard (embebido en el firmware)

**Ejemplo**:
```bash
curl http://zaque.local/
```

### GET /dashboard
Alternativa para obtener el dashboard (misma respuesta que GET /).

### GET /api/status
Estado general del sistema MAIN.

**Respuesta**:
```json
{
  "status": "ok",
  "role": "main",
  "node_id": "main",
  "firmware": "0.2.0",
  "uptime_seconds": 3600,
  "connected_sensors": 2,
  "sd_available_mb": 29000,
  "wifi_signal_strength": -67
}
```

**Ejemplo**:
```bash
curl http://zaque.local/api/status
```

### GET /api/nodes
Lista de todos los nodos conocidos (MAIN + SENSOR registrados).

**Respuesta**:
```json
{
  "nodes": [
    {
      "node_id": "main",
      "node_name": "Nodo principal",
      "role": "main",
      "last_measurement": "2026-05-31T16:20:00",
      "battery_percent": 91,
      "is_active": true
    },
    {
      "node_id": "zona_2",
      "node_name": "Lote de maiz",
      "role": "sensor",
      "last_measurement": "2026-05-31T16:19:55",
      "battery_percent": 83,
      "is_active": true
    }
  ]
}
```

**Ejemplo**:
```bash
curl http://zaque.local/api/nodes
```

### GET /api/measurements/latest
Última medición registrada de cada nodo.

**Respuesta**: (Archivo `/sd/latest.json` del MAIN)
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

**Ejemplo**:
```bash
curl http://zaque.local/api/measurements/latest
```

### GET /api/history?node_id=zona_2
Histórico de mediciones de un nodo específico (últimas N registros).

**Parámetros**:
- `node_id`: ID del nodo (requerido)
- `limit`: Número de registros (opcional, default: 100)

**Respuesta**:
```json
{
  "node_id": "zona_2",
  "measurements": [
    {
      "timestamp": "2026-05-31T16:20:05",
      "lat": 4.7112,
      "lon": -74.0721,
      "soil_temperature": 22.8,
      "soil_humidity": 64.0,
      "electrical_conductivity": 820,
      "ph": 6.2,
      "nitrogen": 34,
      "phosphorus": 18,
      "potassium": 41,
      "battery_percent": 83
    },
    {
      "timestamp": "2026-05-31T15:50:10",
      "lat": 4.7112,
      "lon": -74.0721,
      "soil_temperature": 22.5,
      "soil_humidity": 62.0,
      "electrical_conductivity": 810,
      "ph": 6.3,
      "nitrogen": 35,
      "phosphorus": 19,
      "potassium": 40,
      "battery_percent": 85
    }
  ]
}
```

**Ejemplo**:
```bash
curl "http://zaque.local/api/history?node_id=zona_2&limit=50"
```

### POST /api/measurements
Recibir medición de un nodo SENSOR.

**Headers**:
- `Content-Type: application/json`

**Body Request** (enviado por SENSOR):
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

**Body Response** (del MAIN):
```json
{
  "status": "ok",
  "stored": true,
  "recommendation": "Humedad adecuada y pH dentro de rango aceptable.",
  "server_time": "2026-05-31T16:20:05"
}
```

**Códigos de respuesta**:
- `200 OK`: Medición recibida y almacenada
- `400 Bad Request`: JSON inválido o campos faltantes
- `401 Unauthorized`: api_key incorrecta
- `500 Internal Server Error`: Error al guardar

**Ejemplo con curl**:
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
    "soil_temperature": 22.8,
    "soil_humidity": 64.0,
    "electrical_conductivity": 820,
    "ph": 6.2,
    "nitrogen": 34,
    "phosphorus": 18,
    "potassium": 41,
    "battery_percent": 83,
    "firmware_version": "0.2.0"
  }'
```

### GET /download/measurements.csv
Descargar archivo CSV con todas las mediciones guardadas.

**Respuesta**:
- Content-Type: `text/csv`
- Body: Archivo CSV en formato:
  ```csv
  timestamp,node_id,node_name,lat,lon,soil_temperature,soil_humidity,electrical_conductivity,ph,nitrogen,phosphorus,potassium,battery_percent,recommendation
  2026-05-31T16:20:00,zona_2,Lote de maiz,4.7112,-74.0721,22.8,64,820,6.2,34,18,41,83,"Humedad adecuada"
  ```

**Ejemplo**:
```bash
curl http://zaque.local/download/measurements.csv > measurements.csv
```

## Validación y Errores

### Validación de entrada (POST /api/measurements)

Campos requeridos:
- `api_key` (string)
- `node_id` (string, max 32 chars)
- `node_name` (string, max 64 chars)
- `role` (string: "sensor" or "main")
- `timestamp` (string ISO 8601)
- `lat` (float, -90 a 90)
- `lon` (float, -180 a 180)
- `soil_humidity` (float, 0-100)
- `ph` (float, 0-14)
- `nitrogen` (int, 0-300)
- `phosphorus` (int, 0-300)
- `potassium` (int, 0-300)
- `battery_percent` (int, 0-100)

Campos opcionales:
- `soil_temperature` (float)
- `electrical_conductivity` (int)
- `firmware_version` (string)

### Códigos de error

```json
{
  "status": "error",
  "code": "INVALID_JSON",
  "message": "JSON malformed or incomplete"
}
```

Posibles códigos de error:
- `INVALID_JSON`: JSON inválido o incompleto
- `MISSING_FIELD`: Campo requerido ausente
- `INVALID_API_KEY`: api_key incorrecta
- `INVALID_VALUE`: Valor fuera de rango
- `STORAGE_ERROR`: Error al guardar en SD
- `INTERNAL_ERROR`: Error interno del servidor

## Seguridad

### API Key

Por defecto, se requiere api_key en POST /api/measurements.

Valor por defecto: `ZAQUE_LOCAL_KEY`

Se recomienda:
1. Cambiar en `config.h` antes de compilar
2. Futuro: Implementar JWT
3. Futuro: Implementar certificados TLS (cuando haya internet)

### HTTPS

No está implementado en MVP (se usa HTTP sobre red local).

Futuro: Agregar soporte HTTPS cuando haya conectividad a internet.

### Validación

- Se validan todos los valores numéricos
- Se validan coordenadas GPS
- Se valida timestamp ISO 8601
- Se truncan strings a longitud máxima

## Rate Limiting

No hay rate limiting en MVP. Futuro: Implementar si es necesario.

## Versionado

Versión de API: 1.0

Se puede consultar en GET /api/status.

Cambios futuros que requieren versionado:
- Agregar nuevos campos obligatorios
- Cambiar estructura de respuesta
- Cambiar códigos de error

## Ejemplos de Integración

### Python

```python
import requests
import json
from datetime import datetime

def send_measurement(main_ip, measurement):
    url = f"http://{main_ip}/api/measurements"
    headers = {"Content-Type": "application/json"}
    
    measurement['timestamp'] = datetime.now().isoformat()
    measurement['api_key'] = 'ZAQUE_LOCAL_KEY'
    
    response = requests.post(url, json=measurement, headers=headers)
    return response.json()

# Ejemplo
meas = {
    "node_id": "zona_2",
    "node_name": "Lote de maiz",
    "role": "sensor",
    "lat": 4.7112,
    "lon": -74.0721,
    "soil_humidity": 64.0,
    "ph": 6.2,
    "nitrogen": 34,
    "phosphorus": 18,
    "potassium": 41,
    "battery_percent": 83
}

result = send_measurement("192.168.43.50", meas)
print(result)
```

### JavaScript

```javascript
async function sendMeasurement(mainIP, measurement) {
    const url = `http://${mainIP}/api/measurements`;
    
    measurement.timestamp = new Date().toISOString();
    measurement.api_key = 'ZAQUE_LOCAL_KEY';
    
    const response = await fetch(url, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(measurement)
    });
    
    return await response.json();
}

// Ejemplo
const meas = {
    node_id: "zona_2",
    node_name: "Lote de maiz",
    role: "sensor",
    lat: 4.7112,
    lon: -74.0721,
    soil_humidity: 64.0,
    ph: 6.2,
    nitrogen: 34,
    phosphorus: 18,
    potassium: 41,
    battery_percent: 83
};

sendMeasurement("192.168.43.50", meas).then(console.log);
```

## Testing

Para probar el API sin ESP32:

```bash
# Obtener estado
curl http://zaque.local/api/status

# Enviar medición fake
curl -X POST http://zaque.local/api/measurements \
  -H "Content-Type: application/json" \
  -d @measurement.json

# Ver última medición
curl http://zaque.local/api/measurements/latest

# Descargar CSV
curl http://zaque.local/download/measurements.csv > data.csv
```
