# Zaque 🌱

Proyecto de investigación sobre un dispositivo inteligente de monitoreo ambiental de suelos para comunidades campesinas de Colombia. Los nodos ESP32 miden variables del suelo (temperatura, humedad, conductividad, pH, NPK) y envían los datos vía MQTT a una base de datos central, con el objetivo de que los agricultores puedan tomar decisiones informadas sobre cuándo y qué sembrar.

---

## Estado del proyecto

🚧 **En desarrollo activo**

| Módulo | Estado | Descripción |
|---|---|---|
| ESP32 Firmware | ✅ Funcional | Nodo sensor (ESP-NOW + deep sleep) y nodo gateway (WiFi + MQTT) |
| Base de datos | ✅ Funcional | Servidor Node.js que recibe datos MQTT y los persiste en MySQL |
| API | 🔜 Próximamente | REST API para consulta de datos históricos |
| Frontend | 🔜 Próximamente | Dashboard web de visualización |
| Backend | 🔜 Próximamente | Lógica de negocio y procesamiento de datos |

---

## Arquitectura

```
[ESP32 Emisor]  ──ESP-NOW──►  [ESP32 Receptor]  ──MQTT──►  [Servidor Node.js]  ──►  [MySQL]
  (sensor NPK)                  (gateway WiFi)              (Base de datos/)
```

Los nodos emisores operan en modo **deep sleep** de 30 minutos para maximizar la vida útil de la batería. El receptor permanece activo y hace de puente entre la red ESP-NOW local y el broker MQTT en la nube.

---

## Estructura del repositorio

```
Zaque/
├── ESP32/                             # Firmware para microcontroladores ESP32
│   ├── ESPNOW_bajo_consumo_emisor.ino # Nodo sensor: lee RS485/Modbus, envía por ESP-NOW y entra en deep sleep
│   ├── Receptor_con_MQTT_y_Bateria.ino# Nodo gateway: recibe ESP-NOW y publica en MQTT
│   └── secrets_example.h              # Plantilla de credenciales (copiar a secrets.h)
│
├── Base de datos/                     # Servidor MQTT → MySQL (Node.js)
│   ├── config/db.js                   # Pools de conexión MySQL (local y remota)
│   ├── server.js                      # Express + cliente MQTT
│   └── .env.example                   # Plantilla de variables de entorno
│
├── Api/                               # (Próximamente) REST API
├── Frontend/                          # (Próximamente) Dashboard web
└── Backend/                           # (Próximamente) Lógica de negocio
```

---

## Variables medidas

| Variable | Unidad | Sensor |
|---|---|---|
| Temperatura del suelo | °C | RS485/Modbus (NPK) |
| Humedad del suelo | %RH | RS485/Modbus (NPK) |
| Conductividad eléctrica | µS/cm | RS485/Modbus (NPK) |
| pH | — | RS485/Modbus (NPK) |
| Nitrógeno (N) | mg/kg | RS485/Modbus (NPK) |
| Fósforo (P) | mg/kg | RS485/Modbus (NPK) |
| Potasio (K) | mg/kg | RS485/Modbus (NPK) |
| Nivel de batería | % | ADC interno ESP32 |

---

## Configuración

### Base de datos (Node.js)

```bash
cd "Base de datos"
npm install
cp .env.example .env   # Completar con tus credenciales
node server.js
```

### ESP32 (Arduino IDE)

1. Copiar `ESP32/secrets_example.h` → `ESP32/secrets.h`
2. Editar `secrets.h` con tus credenciales de WiFi y MQTT
3. Cargar los sketches con Arduino IDE

**Librerías requeridas:**
- `ModbusMaster`
- `PubSubClient`
- Framework Arduino para ESP32

---

## Hardware

- **ESP32** (×2): Microcontroladores principales
- **Sensor NPK (RS485/Modbus)**: Mide temperatura, humedad, conductividad, pH, N, P y K del suelo
- **Módulo HW-519**: Interfaz RS485 para comunicación con el sensor

---

## Licencia

Ver [LICENSE](LICENSE)

