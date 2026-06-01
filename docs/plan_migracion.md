# Plan de Migración - Zaque Offline Multinodo

## Visión General

Migración de Zaque de una arquitectura basada en **ESP-NOW + MQTT + servidor remoto** a una arquitectura **WiFi local + HTTP + almacenamiento local**.

**Cambio principal**: De depender de internet y servidor remoto → Sistema autónomo local para zonas rurales.

---

## Fases de Implementación

### Fase 1: MAIN Autónomo (Alcance MVP)

**Objetivo**: Un nodo MAIN mide, almacena en SD y levanta dashboard web local.

**Duración estimada**: 1-2 semanas

**Tasks**:
- [ ] Implementar `sensor_reader.cpp` – Lectura RS485/Modbus
- [ ] Implementar `gps_reader.cpp` – Lectura GPS TinyGPS+
- [ ] Implementar `sd_logger.cpp` – Logging en microSD (CSV + JSON)
- [ ] Implementar `wifi_manager.cpp` – Conexión WiFi local
- [ ] Implementar `web_server.cpp` – Servidor HTTP básico
- [ ] Implementar `node_registry.cpp` – Registro interno de nodos
- [ ] Implementar `recommendations.cpp` – Motor de reglas simple
- [ ] Validar funcionamiento en hardware real
- [ ] Documentar procedimiento de carga

**Criterios de éxito**:
- [ ] MAIN se conecta a WiFi
- [ ] MAIN levanta servidor en puerto 80
- [ ] Dashboard accesible desde `http://zaque.local` (mDNS)
- [ ] MAIN toma medición de sensor local
- [ ] MAIN lee GPS local
- [ ] MAIN guarda medición en `/sd/measurements.csv`
- [ ] MAIN actualiza `/sd/latest.json`
- [ ] Dashboard muestra medición en tiempo real
- [ ] Recomendaciones agrícolas se muestran en dashboard

**Salida**:
- Firmware MAIN compilable y funcional
- Dashboard web local visible desde celular
- Datos almacenados en SD

---

### Fase 2: API Local del MAIN

**Objetivo**: MAIN expone endpoint HTTP para recibir mediciones de sensores.

**Duración estimada**: 1 semana

**Tasks**:
- [ ] Implementar `POST /api/measurements` en web_server.cpp
- [ ] Parsear JSON recibido con ArduinoJson
- [ ] Validar campos requeridos (node_id, humedad, pH, NPK, batería, etc.)
- [ ] Guardar medición en `/sd/measurements.csv`
- [ ] Actualizar `/sd/latest.json` con nueva medición
- [ ] Generar recomendación y incluir en respuesta
- [ ] Implementar validación de API key
- [ ] Implementar retry logic (timeout, errores HTTP)
- [ ] Documentar endpoints en `protocolo_api_local.md`
- [ ] Crear script de prueba (Python/Node/curl)

**Criterios de éxito**:
- [ ] Curl puede enviar JSON POST y recibe respuesta válida
- [ ] Medición se almacena en SD
- [ ] Dashboard MAIN actualiza cuando recibe POST
- [ ] API rechaza requests sin API key
- [ ] API rechaza JSON inválido con error 400
- [ ] Recomendación se retorna en respuesta

**Salida**:
- API local completamente funcional
- MAIN puede recibir y procesar mediciones remotas
- Documentación de endpoints

---

### Fase 3: Firmware SENSOR

**Objetivo**: Nodo SENSOR mide, guarda localmente en SD y envía al MAIN.

**Duración estimada**: 1-2 semanas

**Tasks**:
- [ ] Compilar config.h con `DEVICE_ROLE_SENSOR`
- [ ] Implementar `http_client.cpp` – Cliente HTTP
- [ ] Implementar mDNS lookup para encontrar `zaque.local`
- [ ] Implementar fallback a IP fija si mDNS falla
- [ ] Serializar medición a JSON
- [ ] Enviar POST a `/api/measurements`
- [ ] Manejar respuesta HTTP (éxito/error)
- [ ] Si falla: guardar en `/sd/pending_queue.csv`
- [ ] Implementar reintento automático (exponential backoff)
- [ ] Implementar deep sleep por `MEASUREMENT_INTERVAL_MINUTES`
- [ ] Optimizar para bajo consumo de batería
- [ ] Documentar procedimiento

**Criterios de éxito**:
- [ ] SENSOR conecta a WiFi
- [ ] SENSOR toma medición local
- [ ] SENSOR encuentra MAIN por mDNS
- [ ] SENSOR envía medición por HTTP POST
- [ ] MAIN recibe y almacena
- [ ] Dashboard MAIN muestra medición de SENSOR
- [ ] Si MAIN no está disponible, SENSOR guarda en pending_queue
- [ ] SENSOR entra en deep sleep
- [ ] Batería dura 1-3 meses

**Salida**:
- Firmware SENSOR compilable y funcional
- SENSOR envía datos al MAIN exitosamente
- Dato visible en dashboard MAIN
- Bajo consumo de batería validado

---

### Fase 4: Múltiples Sensores

**Objetivo**: Escalar a 3+ nodos conectados simultáneamente.

**Duración estimada**: 1 semana

**Tasks**:
- [ ] Compilar 3 instancias de SENSOR con diferentes NODE_ID
- [ ] Cargar en 3 ESP32 diferentes
- [ ] Conectar todos a mismo WiFi
- [ ] Verificar que todos se descubren mutuamente
- [ ] Verificar que MAIN recibe de todos simultáneamente
- [ ] Dashboard muestra todos los nodos
- [ ] Prueba de carga: 10+ mediciones simultáneas
- [ ] Validar integridad de datos en SD
- [ ] Documentar troubleshooting

**Criterios de éxito**:
- [ ] 3 sensores conectados al MAIN
- [ ] Cada sensor tiene su propia tarjeta SD
- [ ] Dashboard muestra datos de todos
- [ ] No hay conflictos de red
- [ ] MAIN no se satura

**Salida**:
- Sistema escalable a múltiples nodos
- Documentación de configuración multi-nodo

---

### Fase 5: Recomendaciones Agrícolas

**Objetivo**: Motor de reglas simple con recomendaciones por zona.

**Duración estimada**: 1 semana

**Tasks**:
- [ ] Expandir `recommendations.cpp` con más reglas
- [ ] Definir umbrales por cultivo/zona (configurable)
- [ ] Implementar niveles de alerta (OK / ATENCIÓN / CRÍTICO)
- [ ] Almacenar histórico de recomendaciones en `/sd/recommendations.csv`
- [ ] Dashboard muestra nivel de alerta por color
- [ ] Validar recomendaciones con agrónomo
- [ ] Documentar reglas en `/docs/recomendaciones.md`

**Criterios de éxito**:
- [ ] Reglas cubren: humedad, pH, NPK, batería, temperatura
- [ ] Recomendaciones son accionables
- [ ] Histórico se almacena correctamente
- [ ] Dashboard diferencia niveles de alerta visualmente

**Salida**:
- Motor de recomendaciones funcional
- Documentación de reglas

---

### Fase 6: Descubrimiento Automático

**Objetivo**: SENSOR encuentra MAIN sin IP fija.

**Duración estimada**: 1 semana

**Tasks**:
- [ ] Implementar mDNS en MAIN (ESPmDNS library)
- [ ] Implementar mDNS resolver en SENSOR
- [ ] Implementar fallback: UDP broadcast discovery
- [ ] Implementar fallback: IP configurada en config.h
- [ ] Validar en diferentes redes WiFi
- [ ] Documentar proceso de discovery

**Criterios de éxito**:
- [ ] SENSOR encuentra MAIN por mDNS en la mayoría de casos
- [ ] Fallback funciona si mDNS falla
- [ ] Documento de troubleshooting si discovery falla

**Salida**:
- Descubrimiento automático de MAIN
- Sistema más robusto

---

### Fase 7: Sincronización Opcional con Nube

**Objetivo**: MAIN puede sincronizar con servidor remoto cuando haya internet (futuro).

**Duración estimada**: 2-3 semanas

**Tasks**:
- [ ] Crear módulo `cloud_sync.cpp`
- [ ] Implementar cliente MQTT
- [ ] Implementar cliente REST API
- [ ] Detectar disponibilidad de internet (ping)
- [ ] Subir CSV/JSON cuando haya conexión
- [ ] Implementar sincronización en background
- [ ] Validar sin romper modo offline
- [ ] Documentación

**Criterios de éxito**:
- [ ] Sistema funciona igual sin internet (offline first)
- [ ] Cuando hay internet, sube datos automáticamente
- [ ] No hay conflictos de datos
- [ ] Dashboard remoto opcional en nube

**Salida**:
- Sistema completamente autónomo + opción de sincronización
- Backend Node.js preparado para recibir datos

---

## Timeline Propuesto

```
Semana 1:     Fase 1 (MAIN autónomo)
Semana 2:     Fase 2 (API local)
Semana 3:     Fase 3 (SENSOR)
Semana 4:     Fase 4 (Múltiples sensores)
Semana 5:     Fase 5 (Recomendaciones)
Semana 6:     Fase 6 (Descubrimiento automático)
Semanas 7-9:  Fase 7 (Sincronización con nube)
Semana 10:    Integración, testing, documentación
```

**Total estimado**: 10-12 semanas para MVP + cloud.

---

## Dependencias Entre Fases

```
Fase 1 (MAIN autónomo)
    ↓
Fase 2 (API local del MAIN)
    ↓
Fase 3 (Firmware SENSOR)
    ↓
Fase 4 (Múltiples sensores)
    ↓
Fase 5 (Recomendaciones)

Fase 6 (Descubrimiento) [paralelo a Fase 4-5]
Fase 7 (Nube) [paralelo a otras, independiente]
```

---

## Criterios de Éxito Global

Al final de todas las fases:

- [ ] 1 nodo MAIN + 3+ nodos SENSOR funcionando
- [ ] Dashboard accesible desde celular en red local
- [ ] Todos los datos almacenados en microSD
- [ ] Sistema funciona 100% sin internet
- [ ] Baterías duran 1-3 meses
- [ ] Documentación completa
- [ ] Código en GitHub público
- [ ] Compilable en Arduino IDE estándar
- [ ] No requiere dependencias externas complicadas
- [ ] Testeado en campo con agricultores

---

## Riesgos Mitigados

| Riesgo | Impacto | Mitigación |
|--------|---------|-----------|
| Conflicto GPIO 13 | Alto | Usar GPIO13 MUX (GPS en init, SD en medición) |
| Consumo WiFi alto | Alto | Deep sleep agresivo en SENSOR |
| SD corrupción | Medio | Escribir cerrar correctamente |
| Dashboard pesado | Medio | HTML/CSS/JS <100KB |
| GPS sin señal | Medio | Usar última ubicación válida |
| Pérdida conexión MAIN | Medio | Guardar pending_queue en SD |
| Límite de dispositivos | Medio | Máximo 10-15 nodos por hotspot |
| Memoria ESP32 limitada | Bajo | JSON pequeños, código eficiente |

---

## Notas Importantes

1. **Prioridad MAIN**: Asegurar que MAIN funciona primero (base de todo)
2. **Testing en campo**: Validar con agricultores desde Fase 1
3. **Documentación**: Escribir en paralelo a implementación
4. **Open Source**: Todo público en GitHub
5. **Bajo costo**: Hardware accesible (<$50 USD por nodo)
6. **Sostenibilidad**: Que otros puedan mantener y extender

---

## Librerías Requeridas

```
Arduino IDE → Manage Libraries:

- ModbusMaster (RS485 Modbus)
- TinyGPSPlus (GPS NEO-6M)
- SD (tarjeta microSD)
- SPI (comunicación SPI)
- WiFi (WiFi ESP32)
- WebServer (servidor HTTP)
- HTTPClient (cliente HTTP)
- ArduinoJson (procesamiento JSON)
- ESPmDNS (mDNS multicast)
```

---

## Próximos Pasos Inmediatos

1. ✅ Crear estructura de carpetas
2. ✅ Crear headers base
3. ✅ Crear config.h unificado
4. ✅ Crear firmware skeleton (zaque_device.ino)
5. ✅ Crear dashboard HTML embebido
6. ⏭️ **Fase 1 - Implementar sensor_reader.cpp**
7. ⏭️ **Fase 1 - Implementar gps_reader.cpp**
8. ⏭️ **Fase 1 - Implementar sd_logger.cpp**
9. ...
