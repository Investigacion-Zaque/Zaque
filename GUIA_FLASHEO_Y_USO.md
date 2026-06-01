# 🚀 GUÍA COMPLETA: Cómo Flashear Zaque y Acceder a la Web

## 📋 Requisitos Previos

### Hardware Necesario
- **Placa ESP32 DEVKIT** (o compatible)
- **Cable USB** (tipo A → micro-B para programación)
- **PC/Laptop** con USB disponible
- **Sensor RS485/Modbus NPK** (opcional para datos reales)
- **Módulo GPS NEO-6M** (opcional)
- **Tarjeta microSD** (recomendado)
- **Batería** (para pruebas completas)

### Software Necesario
- **Arduino IDE 1.8.19+** (descargar de arduino.cc)
- **Drivers CH340** (para ESP32 DEVKIT)
- **Git** (opcional, para clonar repositorio)

---

## 🔧 PASO 1: Instalar Arduino IDE y Drivers

### 1.1 Instalar Arduino IDE
```bash
# Windows/Mac: Descargar de https://www.arduino.cc/en/software
# Linux:
wget https://downloads.arduino.cc/arduino-1.8.19-linux64.tar.xz
tar xf arduino-1.8.19-linux64.tar.xz
cd arduino-1.8.19
./install.sh
```

### 1.2 Instalar Soporte para ESP32
1. Abrir Arduino IDE
2. Ir a: **Archivo** → **Preferencias**
3. En "URL de gestor de tarjetas adicionales", agregar:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Aceptar y cerrar
5. Ir a: **Herramientas** → **Placa** → **Gestor de tarjetas**
6. Buscar "ESP32" por Espressif Systems
7. Instalar la última versión
8. Seleccionar placa: **Herramientas** → **Placa** → **ESP32 Dev Module**

### 1.3 Instalar Librerías Necesarias
En Arduino IDE, ir a: **Sketch** → **Incluir librería** → **Gestor de librerías**

Buscar e instalar:
- `ModbusMaster` (Rob Tillaart)
- `TinyGPSPlus` (Mikal Hart)
- `ArduinoJson` (Benoit Blanchon) - versión 6.x
- `WebServer` (incluido con ESP32)

---

## 🎯 PASO 2: Configurar el Firmware

### 2.1 Abrir el Proyecto
1. Descargar el repositorio Zaque
2. En Arduino IDE: **Archivo** → **Abrir**
3. Navegar a: `/home/ingeniero/Desktop/Zaque/firmware/zaque_device/`
4. Seleccionar: `zaque_device.ino`

### 2.2 Configurar como MAIN (Nodo Principal)
Editar `config.h` (en el mismo directorio):

```cpp
// Línea 14-15: Asegurar que es MAIN
#define DEVICE_ROLE DEVICE_ROLE_MAIN
// #define DEVICE_ROLE DEVICE_ROLE_SENSOR

// Línea 19-20: Identificación
#define NODE_ID "main"
#define NODE_NAME "Nodo principal - Casa finca"

// Línea 27-28: WiFi local (CAMBIAR CON TUS DATOS)
#define WIFI_SSID "NombreZonaMovil"        // ← Cambiar al nombre de tu zona móvil
#define WIFI_PASSWORD "ClaveZonaMovil"     // ← Cambiar a tu contraseña WiFi
```

### 2.3 Configurar Conexión Serial (Arduino IDE)
1. Conectar ESP32 a la PC con cable USB
2. En Arduino IDE:
   - **Herramientas** → **Puerto** → Seleccionar puerto COM donde aparezca ESP32
   - **Herramientas** → **Velocidad en bauds** → `115200`
   - **Herramientas** → **Placa** → `ESP32 Dev Module`

---

## ⚡ PASO 3: Compilar y Cargar (Flashear)

### 3.1 Compilar
```
En Arduino IDE, presionar: Ctrl+R  (o Sketch → Verificar)
```

Esperar a que compile. Si hay errores:
- Revisar que todas las librerías estén instaladas
- Revisar que config.h esté en el mismo directorio que zaque_device.ino

### 3.2 Flashear a la Placa
```
En Arduino IDE, presionar: Ctrl+U  (o Sketch → Cargar)
```

**Proceso de carga:**
1. Arduino IDE muestra "Esperando a la carga del puerto..."
2. **PRESIONAR EL BOTÓN "BOOT" del ESP32 y mantener presionado**
3. Esperar unos segundos
4. Arduino IDE muestra barra de progreso
5. Cuando termine, soltar el botón BOOT

> ⚠️ Si falla, revisar que el puerto COM sea correcto

---

## 🌐 PASO 4: Verificar Carga Exitosa

### 4.1 Monitorear Puerto Serial
1. En Arduino IDE: **Herramientas** → **Monitor Serial**
2. Deberías ver logs como:
```
=== ZAQUE Initialization ===
Device Role: MAIN
Node ID: main
Firmware: 0.2.0
→ Initializing soil sensor...
→ Initializing GPS...
→ Initializing SD card...
→ Connecting to WiFi...
✓ Initialization complete
✓ WiFi connected
✓ mDNS: main.local
✓ Web server started on port 80
  → http://zaque.local/
```

Si ves estos mensajes, **¡está funcionando!** ✅

### 4.2 Solucionar Problemas Comunes

| Problema | Causa | Solución |
|----------|-------|----------|
| "Esperando carga..." infinitamente | Puerto incorrecto | Revisar puerto COM en Herramientas |
| Error compilación | Falta librería | Instalar desde Gestor de librerías |
| Error "board not found" | ESP32 no detectado | Instalar drivers CH340 |
| WiFi no conecta | Credenciales incorrectas | Revisar WIFI_SSID y WIFI_PASSWORD en config.h |

---

## 🌍 PASO 5: Acceder al Dashboard Web

### Opción A: Via mDNS (Recomendado)
1. Encender ESP32 (ya debería estar encendido)
2. Abrir navegador en **celular o PC**
3. Ingresar dirección:
   ```
   http://zaque.local
   ```

### Opción B: Via IP Directa
Si mDNS no funciona:
1. En el Monitor Serial, buscar línea:
   ```
   IP: 192.168.x.x
   ```
2. Copiar esa IP
3. En navegador, ingresar:
   ```
   http://192.168.43.50    (o la IP que viste)
   ```

### Opción C: Descubrir IP via Serial
```bash
# En terminal:
espressif-idf@esp32 > idf.py monitor

# Verás algo como:
# WiFi connected
# IP: 192.168.43.123
```

---

## 📱 PASO 6: Usar el Dashboard

### Pantalla Principal
Cuando ingreses a `http://zaque.local`, verás:

```
┌─────────────────────────────────────┐
│  🌾 Zaque                           │
│  Sistema local de monitoreo         │
│  Actualizado: 17:30:45              │
├─────────────────────────────────────┤
│  Nodos Totales: 1                   │
│  Nodos Activos: 1                   │
│  Última Medición: 17:30:00          │
├─────────────────────────────────────┤
│ 🔋 MAIN - Casa finca                │
│ Humedad: 58%                        │
│ pH: 6.4                             │
│ NPK: 35 / 20 / 40                   │
│ Batería: 91%                        │
│ ✓ Condiciones estables              │
│ [🔄 Actualizar] [📥 Descargar CSV]  │
└─────────────────────────────────────┘
```

### Funciones Disponibles

| Botón | Función | Resultado |
|-------|---------|-----------|
| 🔄 Actualizar | Refresca datos | Recargar mediciones actuales |
| 📥 Descargar CSV | Descarga archivo | measurements.csv con histórico |

---

## 🔌 PASO 7: Crear Nodo SENSOR (Opcional)

Si quieres agregar más puntos de medición:

### 7.1 Configurar Para SENSOR
Editar `config.h` nuevamente:

```cpp
// Cambiar rol a SENSOR
#define DEVICE_ROLE DEVICE_ROLE_SENSOR

// Identificar este sensor
#define NODE_ID "zona_2"
#define NODE_NAME "Lote de maiz"

// WiFi sigue igual (misma zona móvil)
#define WIFI_SSID "NombreZonaMovil"
#define WIFI_PASSWORD "ClaveZonaMovil"
```

### 7.2 Cargar en Segundo ESP32
1. Tomar un **segundo ESP32**
2. Repetir Paso 3 (Compilar y Flashear)
3. El SENSOR automáticamente:
   - Se conectará a WiFi
   - Encontrará el MAIN via mDNS (`zaque.local`)
   - Tomará medición
   - Enviará al MAIN
   - Entrará en deep sleep

### 7.3 Ver en Dashboard
Volver a `http://zaque.local` y verás:
```
🌾 Zaque

Nodos Totales: 2
Nodos Activos: 2
...

🔋 MAIN - Casa finca
Humedad: 58%
...

🔋 Zona 2 - Lote de maiz
Humedad: 64%
...
```

---

## 📊 PASO 8: Monitorear Datos

### Ver Datos en CSV
El ESP32 guarda automáticamente en SD:
- **Ruta**: `/sd/measurements.csv`
- **Acceso**: Descargar desde botón 📥 en dashboard

### Contenido CSV
```csv
timestamp,node_id,node_name,lat,lon,soil_temperature,soil_humidity,...
1685572800000,main,Casa finca,4.7110,-74.0710,22.5,58,...
1685572860000,zona_2,Lote de maiz,4.7112,-74.0721,22.8,64,...
```

### Ver Logs en Serial
Monitor Serial muestra eventos en tiempo real:
```
→ Taking measurement...
✓ Sensor read: T=22.5 H=58% pH=6.4 NPK=35/20/40
✓ Saving to SD...
✓ Main measurement recorded
```

---

## 🔗 PASO 9: Probar API con CURL (Avanzado)

### Enviar Medición Manual
```bash
curl -X POST http://zaque.local/api/measurements \
  -H "Content-Type: application/json" \
  -d '{
    "api_key": "ZAQUE_LOCAL_KEY",
    "node_id": "zona_3",
    "node_name": "Cafetal",
    "role": "sensor",
    "timestamp": "2026-05-31T17:30:00",
    "lat": 4.7120,
    "lon": -74.0730,
    "soil_humidity": 52.0,
    "ph": 5.8,
    "nitrogen": 28,
    "phosphorus": 16,
    "potassium": 32,
    "battery_percent": 71,
    "firmware_version": "0.2.0"
  }'
```

### Respuesta Esperada
```json
{
  "status": "ok",
  "stored": true,
  "recommendation": "Suelo ácido: considerar enmienda.",
  "server_time": "2026-05-31T17:30:05"
}
```

---

## ⚙️ CONFIGURACIONES AVANZADAS

### Cambiar Intervalo de Medición
En `config.h`, línea 83:
```cpp
#define MEASUREMENT_INTERVAL_MINUTES 30  // Cambiar a 15, 60, etc.
```

### Cambiar SSID/Password WiFi (Sin Recompilar)
Próxima versión: Portal de configuración en ESP32

### Habilitar Debug Completo
En `config.h`, línea 216:
```cpp
#define ENABLE_DEBUG_SERIAL true  // Mostrar todos los logs
```

---

## 🆘 SOLUCIÓN DE PROBLEMAS

### "No puedo acceder a zaque.local"
```bash
# Opción 1: Usar IP en lugar de mDNS
http://192.168.43.50

# Opción 2: Escanear red para encontrar ESP32
nmap -sn 192.168.43.0/24
# Buscar MAC 80:7F:... (Espressif)

# Opción 3: Verificar en Serial Monitor
# Debería mostrar: "✓ mDNS: main.local"
```

### "WiFi conectando infinitamente"
```cpp
// En config.h, verificar:
#define WIFI_SSID "NombreExacto"      // Sensible a mayúsculas
#define WIFI_PASSWORD "PasswordExacto"
```

### "SD card no detectada"
- Revisar conexión de pines SPI (GPIO 5, 13, 18, 19)
- Revisar que la SD esté formateada FAT32
- Probar con otra SD

### "Sensor Modbus no funciona"
- Revisar cables RXD2/TXD2 (GPIO 16/17)
- Verificar baudrate: 9600 bps (línea 97 config.h)
- Validar dirección Modbus: 1 (línea 95 config.h)

---

## 📈 PRÓXIMOS PASOS

1. **Agregar segundo ESP32 como SENSOR**
2. **Calibrar sensor NPK según suelo**
3. **Conectar batería para autonomía**
4. **Instalar en campo y testear**
5. **Descarga y análisis de datos en Excel**

---

## 🎓 REFERENCIAS ÚTILES

- **Arduino IDE Docs**: https://docs.arduino.cc/
- **ESP32 Docs**: https://docs.espressif.com/projects/esp-idf/
- **ModbusMaster**: https://github.com/4-20ma/ModbusMaster
- **TinyGPS++**: https://github.com/mikalhart/TinyGPSPlus

---

## ✅ CHECKLIST FINAL

- [ ] Arduino IDE instalado
- [ ] Librerías necesarias instaladas
- [ ] config.h configurado con WiFi correcto
- [ ] ESP32 conectado por USB
- [ ] Compilación exitosa
- [ ] Carga exitosa (sin errores en Monitor Serial)
- [ ] WiFi conectado (ver "✓ WiFi connected" en Monitor)
- [ ] Acceso a http://zaque.local desde navegador
- [ ] Dashboard cargando mediciones
- [ ] Descargar CSV funciona
- [ ] Datos guardados en SD correctamente

---

**¡Listo! Tu Zaque está funcionando. 🚀**

Campesinos sin internet ahora pueden monitorear sus cultivos.

---

*Última actualización: 31 de Mayo de 2026*
*Versión Firmware: 0.2.0 Offline Multinodo*
