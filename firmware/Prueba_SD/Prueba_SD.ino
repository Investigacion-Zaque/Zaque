#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// ===============================
// PINES MICROSD / SPI
// ===============================
#define GPIO_SD_CS   5    // Chip Select
#define GPIO_SD_SCK  23   // Clock
#define GPIO_SD_MOSI 18   // MOSI
#define GPIO_SD_MISO 19   // MISO

// Usamos HSPI para no depender del SPI por defecto
SPIClass sdSPI(HSPI);

// Archivo de prueba
const char* TEST_FILE = "/test_sd.txt";

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listando directorio: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("❌ No se pudo abrir el directorio");
    return;
  }

  if (!root.isDirectory()) {
    Serial.println("❌ No es un directorio");
    return;
  }

  File file = root.openNextFile();

  while (file) {
    if (file.isDirectory()) {
      Serial.print("DIR : ");
      Serial.println(file.name());

      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }

    file = root.openNextFile();
  }
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Escribiendo archivo: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("❌ No se pudo abrir el archivo para escritura");
    return;
  }

  if (file.print(message)) {
    Serial.println("✅ Escritura correcta");
  } else {
    Serial.println("❌ Falló la escritura");
  }

  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Agregando al archivo: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("❌ No se pudo abrir el archivo para agregar datos");
    return;
  }

  if (file.print(message)) {
    Serial.println("✅ Datos agregados correctamente");
  } else {
    Serial.println("❌ Falló al agregar datos");
  }

  file.close();
}

void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Leyendo archivo: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("❌ No se pudo abrir el archivo para lectura");
    return;
  }

  Serial.println("Contenido del archivo:");
  Serial.println("----------------------");

  while (file.available()) {
    Serial.write(file.read());
  }

  Serial.println();
  Serial.println("----------------------");

  file.close();
}

void testSD() {
  Serial.println();
  Serial.println("=================================");
  Serial.println("          TEST MICROSD");
  Serial.println("=================================");

  Serial.println("Inicializando SPI para SD...");
  sdSPI.begin(GPIO_SD_SCK, GPIO_SD_MISO, GPIO_SD_MOSI, GPIO_SD_CS);

  Serial.println("Inicializando tarjeta SD...");

  if (!SD.begin(GPIO_SD_CS, sdSPI)) {
    Serial.println("❌ SD initialization failed!");
    Serial.println();
    Serial.println("Revisa:");
    Serial.println("1) CS -> GPIO5");
    Serial.println("2) SCK -> GPIO23");
    Serial.println("3) MOSI -> GPIO18");
    Serial.println("4) MISO -> GPIO19");
    Serial.println("5) VCC correcto: 3.3V o 5V según módulo");
    Serial.println("6) GND común");
    Serial.println("7) La microSD está formateada en FAT32");
    Serial.println("8) Prueba otra microSD si sigue fallando");
    return;
  }

  Serial.println("✅ SD inicializada correctamente");

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("❌ No hay tarjeta SD insertada");
    return;
  }

  Serial.print("Tipo de tarjeta: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("DESCONOCIDA");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.print("Tamaño tarjeta: ");
  Serial.print(cardSize);
  Serial.println(" MB");

  uint64_t totalBytes = SD.totalBytes() / (1024 * 1024);
  uint64_t usedBytes = SD.usedBytes() / (1024 * 1024);

  Serial.print("Espacio total: ");
  Serial.print(totalBytes);
  Serial.println(" MB");

  Serial.print("Espacio usado: ");
  Serial.print(usedBytes);
  Serial.println(" MB");

  Serial.println();
  listDir(SD, "/", 1);

  Serial.println();
  writeFile(SD, TEST_FILE, "Hola, SD desde ESP32\n");

  appendFile(SD, TEST_FILE, "Linea agregada 1\n");
  appendFile(SD, TEST_FILE, "Linea agregada 2\n");

  Serial.println();
  readFile(SD, TEST_FILE);

  Serial.println();
  listDir(SD, "/", 1);

  Serial.println("✅ TEST SD TERMINADO");
  Serial.println("=================================");
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("ESP32 microSD test");
  Serial.println("Pines:");
  Serial.println("CS   -> GPIO5");
  Serial.println("SCK  -> GPIO23");
  Serial.println("MOSI -> GPIO18");
  Serial.println("MISO -> GPIO19");

  testSD();
}

void loop() {
  // No hacemos nada en loop.
}