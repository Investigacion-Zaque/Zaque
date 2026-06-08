// Librería base de Arduino para ESP32.
// Incluye funciones como Serial, pinMode, digitalWrite, delay, analogRead, etc.
#include <Arduino.h>

// Librería para comunicación Modbus RTU.
// La usamos para preguntarle datos a la sonda TH-NPK por RS485.
#include <ModbusMaster.h>

// ==========================================================
// CONFIGURACIÓN UART2 / RS485
// ==========================================================
// El ESP32 tiene varios UART.
// Serial normalmente usa el USB.
// Serial2 lo usamos para hablar con el módulo RS485 HW-519.

// Pin RX del ESP32.
// Aquí debe conectarse el TXD del módulo HW-519.
#define RXD2 16

// Pin TX del ESP32.
// Aquí debe conectarse el RXD del módulo HW-519.
#define TXD2 17

// Velocidad de comunicación Modbus.
// Esta fue la que te funcionó con tu sonda.
#define RS485_BAUD 4800

// ==========================================================
// CONTROL DE ALIMENTACIÓN
// ==========================================================
// Estos pines los usas para encender/alimentar el módulo o sensor.
// OJO: en tu HW-519 automático NO son DE/RE.
// Solo los estamos usando como control de energía/habilitación.

#define POWER_PIN 2
#define POWER_PIN2 4

// ==========================================================
// TIEMPO ENTRE LECTURAS
// ==========================================================
// Cada cuánto se vuelve a leer la sonda.
// 3000 ms = 3 segundos.
#define READ_INTERVAL_MS 3000

// ==========================================================
// BATERÍA
// ==========================================================
// Pin analógico donde tienes conectado el divisor de tensión de batería.
// A6 depende de la placa; en algunas ESP32 puede mapear a un GPIO específico.
const int analogInPin = A6;

// Valor calculado a partir del ADC.
// Lo dejamos global para imprimirlo luego en el monitor serie.
float sensorValue = 0;

// ==========================================================
// OBJETO MODBUS
// ==========================================================
// Este objeto es el encargado de hacer las peticiones Modbus.
// Con él llamamos funciones como readHoldingRegisters().
ModbusMaster node;

// ==========================================================
// VARIABLES DE LECTURA
// ==========================================================
// Aquí guardamos los últimos datos recibidos de la sonda.

// Estado de la comunicación Modbus.
// 0 = lectura correcta.
// Otro valor = código de error Modbus.
uint8_t status = 255;

// Temperatura del suelo en grados Celsius.
float temperature = 0.0;

// Humedad del suelo en porcentaje.
float humidity = 0.0;

// Conductividad eléctrica en µS/cm.
float conductivity = 0.0;

// pH del suelo.
float ph = 0.0;

// Nitrógeno en mg/kg.
uint16_t N = 0;

// Fósforo en mg/kg.
uint16_t P = 0;

// Potasio en mg/kg.
uint16_t K = 0;

// Porcentaje de batería calculado.
uint16_t bateria = 0;

// ==========================================================
// FUNCIÓN: IMPRIMIR ERRORES MODBUS
// ==========================================================
// Recibe el código de error que devuelve ModbusMaster
// y lo traduce a texto entendible en el monitor serie.
void printModbusError(uint8_t result) {
  Serial.print("Error Modbus decimal: ");
  Serial.println(result);

  Serial.print("Error Modbus HEX: 0x");
  Serial.println(result, HEX);

  switch (result) {
    case node.ku8MBSuccess:
      Serial.println("OK");
      break;

    case node.ku8MBIllegalFunction:
      Serial.println("Función ilegal. Prueba readInputRegisters en vez de readHoldingRegisters.");
      break;

    case node.ku8MBIllegalDataAddress:
      Serial.println("Dirección de registro ilegal.");
      break;

    case node.ku8MBIllegalDataValue:
      Serial.println("Valor de dato ilegal.");
      break;

    case node.ku8MBSlaveDeviceFailure:
      Serial.println("Fallo del dispositivo esclavo.");
      break;

    case node.ku8MBInvalidSlaveID:
      Serial.println("ID de esclavo inválido o no responde.");
      break;

    case node.ku8MBInvalidFunction:
      Serial.println("Función inválida.");
      break;

    case node.ku8MBResponseTimedOut:
      Serial.println("Timeout: la sonda no respondió.");
      break;

    case node.ku8MBInvalidCRC:
      Serial.println("CRC inválido: posible A/B invertido, ruido o baudrate incorrecto.");
      break;

    default:
      Serial.println("Error desconocido.");
      break;
  }
}

// ==========================================================
// FUNCIÓN: LEER BATERÍA
// ==========================================================
// Lee el pin analógico de batería y lo convierte a porcentaje.
// La fórmula viene de tu código funcional anterior.
void readBattery() {
  // Lee el valor ADC y aplica un factor de corrección.
  // analogRead normalmente devuelve un valor entre 0 y 4095 en ESP32.
  sensorValue = analogRead(analogInPin) * 1.695;

  // Convierte el valor calculado a porcentaje.
  // 2048 se toma como 0% y 4096 como 100%.
  int batteryPercent = map(sensorValue, 2048, 4096, 0, 100);

  // Limita el porcentaje para que nunca sea menor que 0 ni mayor que 100.
  if (batteryPercent < 0) batteryPercent = 0;
  if (batteryPercent > 100) batteryPercent = 100;

  // Guarda el resultado en la variable global.
  bateria = batteryPercent;
}

// ==========================================================
// FUNCIÓN: IMPRIMIR DATOS
// ==========================================================
// Muestra en el monitor serie todos los valores guardados
// después de una lectura de la sonda.
void printData() {
  Serial.println();
  Serial.println("========== DATOS SONDA ==========");

  Serial.print("Status Modbus: ");
  Serial.println(status);

  if (status == 0) {
    Serial.println("Estado: OK");
  } else {
    Serial.println("Estado: ERROR");
  }

  Serial.print("Temperatura: ");
  Serial.print(temperature, 1);
  Serial.println(" °C");

  Serial.print("Humedad: ");
  Serial.print(humidity, 1);
  Serial.println(" %");

  Serial.print("Conductividad: ");
  Serial.print(conductivity, 0);
  Serial.println(" µS/cm");

  Serial.print("pH: ");
  Serial.println(ph, 1);

  Serial.print("Nitrógeno N: ");
  Serial.print(N);
  Serial.println(" mg/kg");

  Serial.print("Fósforo P: ");
  Serial.print(P);
  Serial.println(" mg/kg");

  Serial.print("Potasio K: ");
  Serial.print(K);
  Serial.println(" mg/kg");

  Serial.print("Batería: ");
  Serial.print(bateria);
  Serial.println(" %");

  Serial.print("ADC batería calculado: ");
  Serial.println(sensorValue);

  Serial.println("=================================");
}

// ==========================================================
// FUNCIÓN: LEER SONDA
// ==========================================================
// Hace la petición Modbus a la sonda TH-NPK.
// Tu sonda respondió usando Holding Registers,
// desde la dirección 0x0000, leyendo 7 registros.
void readSensor() {
  Serial.println();
  Serial.println("Leyendo sonda TH-NPK por Modbus...");
  Serial.println("Modo: readHoldingRegisters(0x0000, 7)");

  // Lee 7 registros Modbus desde la dirección 0x0000.
  // Registro 0: Humedad
  // Registro 1: Temperatura
  // Registro 2: Conductividad
  // Registro 3: pH
  // Registro 4: Nitrógeno
  // Registro 5: Fósforo
  // Registro 6: Potasio
  uint8_t result = node.readHoldingRegisters(0x0000, 7);

  // Si la lectura fue exitosa...
  if (result == node.ku8MBSuccess) {
    status = 0;

    Serial.println("✅ Lectura Modbus exitosa.");
    Serial.println();

    // Imprime los registros crudos tal como llegan de la sonda.
    // Esto es útil para depurar si algún valor sale raro.
    Serial.println("Registros crudos:");
    for (int i = 0; i < 7; i++) {
      uint16_t value = node.getResponseBuffer(i);

      Serial.print("Registro ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(value);
      Serial.print(" / 0x");
      Serial.println(value, HEX);
    }

    // Convierte los registros crudos a unidades reales.
    // Estos factores vienen del comportamiento de tu sonda:
    // humedad y temperatura vienen multiplicadas por 10.
    // pH también viene multiplicado por 10.
    humidity = node.getResponseBuffer(0) / 10.0f;
    temperature = node.getResponseBuffer(1) / 10.0f;
    conductivity = node.getResponseBuffer(2);
    ph = node.getResponseBuffer(3) / 10.0f;
    N = node.getResponseBuffer(4);
    P = node.getResponseBuffer(5);
    K = node.getResponseBuffer(6);

  } else {
    // Si la lectura falló, guardamos el código de error.
    status = result;

    Serial.println("❌ Falló lectura Modbus.");
    printModbusError(result);

    // Limpia los valores para no mostrar datos viejos como si fueran actuales.
    humidity = 0.0;
    temperature = 0.0;
    conductivity = 0.0;
    ph = 0.0;
    N = 0;
    P = 0;
    K = 0;
  }

  // Lee batería después de intentar leer la sonda.
  readBattery();
}

// ==========================================================
// SETUP
// ==========================================================
// Se ejecuta una sola vez al encender o reiniciar la ESP32.
void setup() {
  // Inicializa el monitor serie por USB.
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=================================");
  Serial.println(" ESP32 + TH-NPK SOLO LECTURA");
  Serial.println(" SIN ESP-NOW / SIN WIFI / SIN SLEEP");
  Serial.println("=================================");

  // Configura los pines de alimentación/habilitación como salida.
  pinMode(POWER_PIN, OUTPUT);
  pinMode(POWER_PIN2, OUTPUT);

  // Enciende el módulo/sensor.
  Serial.println("Encendiendo alimentación de módulo/sensor...");
  digitalWrite(POWER_PIN, HIGH);
  digitalWrite(POWER_PIN2, HIGH);

  // Espera a que la sonda y el módulo RS485 arranquen.
  delay(5000);

  // Inicializa el bus Modbus por Serial2.
  Serial.println("Inicializando Modbus...");
  Serial2.begin(RS485_BAUD, SERIAL_8N1, RXD2, TXD2);

  // Inicia el maestro Modbus.
  // 1 es la dirección Modbus de la sonda.
  node.begin(1, Serial2);

  Serial.print("RXD2 ESP32: GPIO");
  Serial.println(RXD2);

  Serial.print("TXD2 ESP32: GPIO");
  Serial.println(TXD2);

  Serial.print("Baudrate RS485: ");
  Serial.println(RS485_BAUD);

  Serial.println();
  Serial.println("Cableado esperado:");
  Serial.println("HW-519 TXD -> ESP32 GPIO16 RXD2");
  Serial.println("HW-519 RXD -> ESP32 GPIO17 TXD2");
  Serial.println("HW-519 GND -> ESP32 GND");
  Serial.println("HW-519 VCC -> 3.3V o 5V");
  Serial.println("HW-519 A+  -> Sonda A+ / D+");
  Serial.println("HW-519 B-  -> Sonda B- / D-");
  Serial.println("GND fuente sonda -> GND ESP32");
  Serial.println("=================================");

  Serial.println("Setup terminado. Iniciando lecturas continuas...");
}

// ==========================================================
// LOOP
// ==========================================================
// Se ejecuta repetidamente mientras la ESP32 esté encendida.
void loop() {
  // Lee la sonda por Modbus.
  readSensor();

  // Imprime los últimos valores leídos.
  printData();

  Serial.println();
  Serial.print("Esperando ");
  Serial.print(READ_INTERVAL_MS / 1000);
  Serial.println(" segundos para nueva lectura...");

  // Espera antes de volver a leer.
  delay(READ_INTERVAL_MS);
}