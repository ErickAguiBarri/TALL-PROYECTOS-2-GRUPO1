/*
  Historial_Humedad.ino
  ------------------------------------------------------------
  Nodo de sensado: DHT11/DHT22 -> registro persistente en EEPROM
  - Formato de registro (8 bytes):
      uint16_t humedad_x100  (2 bytes)  // humedad * 100 (0..10000)
      uint32_t timestamp_s   (4 bytes)  // seconds since epoch (or boot-based)
      uint16_t crc16         (2 bytes)  // CRC-16 CCITT sobre los 6 primeros bytes
  - Cabecera EEPROM (8 bytes):
      uint16_t magic (0x4844 'HD')      (2 bytes)
      uint8_t  version                  (1 byte)
      uint8_t  reserved                 (1 byte)
      uint16_t record_count             (2 bytes)
      uint16_t write_index              (2 bytes)
  - Ventajas: formato compacto, validación CRC, modo circular.
  - Recomendaciones: limitar frecuencia de escritura a EEPROM (uso de cache RAM y guardado manual),
    usar EEPROM.update() para minimizar escrituras.
  ------------------------------------------------------------
  Requerimientos: librería DHT (instalar) y opcional RTClib si USE_RTC = 1.
  Autor: Cristian Luiyi García Huallpa (adaptado, técnico)
  Fecha: Oct 2025
*/

#include <Arduino.h>
#include <DHT.h>
#include <EEPROM.h>

// ------------------------ CONFIGURACIÓN ------------------------
#define DHTPIN 2
#define DHTTYPE DHT11      // Cambia a DHT22 si usas ese sensor

#define USE_RTC 0          // 0 = no usar RTC; 1 = usar RTC (requiere RTClib)
#if USE_RTC
  #include <Wire.h>
  #include <RTClib.h>
  RTC_DS3231 rtc;
#endif

#define MAX_RAM_CACHE 8    // registros en RAM antes de sincronizar a EEPROM (reduce escrituras)
#define SERIAL_BAUD 9600

// ------------------------ EEPROM LAYOUT ------------------------
const uint16_t EEPROM_MAGIC = 0x4844; // 'H' 'D'
const uint8_t  EEPROM_VERSION = 1;

const int EEPROM_HEADER_ADDR = 0;
const int EEPROM_HEADER_SIZE = 8; // 2 + 1 + 1 + 2 + 2

const int RECORD_SIZE = 8; // 2 + 4 + 2
int EEPROM_MAX_RECORDS = 0; // calculado en setup()
int EEPROM_RECORDS_START = 0; // EEPROM_HEADER_ADDR + EEPROM_HEADER_SIZE

// ------------------------ TIPOS DE DATOS ------------------------
struct HistRecord {
  uint16_t humedad_x100;
  uint32_t timestamp_s;
  uint16_t crc;
};

// Cabecera leída en RAM
struct EepromHeader {
  uint16_t magic;
  uint8_t  version;
  uint8_t  reserved;
  uint16_t record_count; // número total de registros válidos (hasta EEPROM_MAX_RECORDS)
  uint16_t write_index;  // índice circular (0..EEPROM_MAX_RECORDS-1) donde escribir siguiente
} header;

// RAM cache circular (reduce escrituras)
HistRecord ramCache[MAX_RAM_CACHE];
int ramCacheCount = 0;

// Variables DHT
DHT dht(DHTPIN, DHTTYPE);

// Tiempo base (si no hay RTC, user puede setear epoch manualmente con SETTIME)
unsigned long bootMillis = 0;
uint32_t epochOffset_s = 0; // segundos que representan la "hora" en boot (se usa si no hay RTC)

// ------------------------ FUNCIONES AUXILIARES ------------------------

// CRC-16 CCITT (poly 0x1021) - implementación estándar (iterativa)
uint16_t crc16_ccitt(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; ++i) {
    crc ^= ((uint16_t)data[i]) << 8;
    for (uint8_t j = 0; j < 8; ++j) {
      if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
      else crc <<= 1;
    }
  }
  return crc;
}

// lee bloque de EEPROM en buffer
void eepromReadBlock(int addr, uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; ++i) buf[i] = EEPROM.read(addr + i);
}

// escribe bloque con EEPROM.update (solo bytes distintos)
void eepromWriteBlock(int addr, const uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    uint8_t old = EEPROM.read(addr + i);
    if (old != buf[i]) EEPROM.update(addr + i, buf[i]);
  }
}

// ------------------------ MANIPULACIÓN CABECERA ------------------------
void writeHeaderToEEPROM() {
  uint8_t buf[EEPROM_HEADER_SIZE];
  buf[0] = (uint8_t)(header.magic >> 8);
  buf[1] = (uint8_t)(header.magic & 0xFF);
  buf[2] = header.version;
  buf[3] = header.reserved;
  buf[4] = (uint8_t)(header.record_count >> 8);
  buf[5] = (uint8_t)(header.record_count & 0xFF);
  buf[6] = (uint8_t)(header.write_index >> 8);
  buf[7] = (uint8_t)(header.write_index & 0xFF);
  eepromWriteBlock(EEPROM_HEADER_ADDR, buf, EEPROM_HEADER_SIZE);
}

void readHeaderFromEEPROM() {
  uint8_t buf[EEPROM_HEADER_SIZE];
  eepromReadBlock(EEPROM_HEADER_ADDR, buf, EEPROM_HEADER_SIZE);
  header.magic = ((uint16_t)buf[0] << 8) | buf[1];
  header.version = buf[2];
  header.reserved = buf[3];
  header.record_count = ((uint16_t)buf[4] << 8) | buf[5];
  header.write_index = ((uint16_t)buf[6] << 8) | buf[7];
}

// Inicializa cabecera si no existe (magic mismatch)
void initEEPROMHeaderIfNeeded() {
  readHeaderFromEEPROM();
  if (header.magic != EEPROM_MAGIC || header.version != EEPROM_VERSION) {
    header.magic = EEPROM_MAGIC;
    header.version = EEPROM_VERSION;
    header.reserved = 0;
    header.record_count = 0;
    header.write_index = 0;
    writeHeaderToEEPROM();
  } else {
    // sanity-check bounds
    if (header.write_index >= EEPROM_MAX_RECORDS) header.write_index = 0;
    if (header.record_count > EEPROM_MAX_RECORDS) header.record_count = EEPROM_MAX_RECORDS;
  }
}

// ------------------------ MANIPULACIÓN DE REGISTROS EN EEPROM ------------------------
int recordAddressByIndex(int idx) {
  // idx: 0..EEPROM_MAX_RECORDS-1
  return EEPROM_RECORDS_START + (idx % EEPROM_MAX_RECORDS) * RECORD_SIZE;
}

bool writeRecordToEEPROM(int idx, const HistRecord &r) {
  uint8_t buf[RECORD_SIZE];
  buf[0] = (uint8_t)(r.humedad_x100 >> 8);
  buf[1] = (uint8_t)(r.humedad_x100 & 0xFF);
  buf[2] = (uint8_t)(r.timestamp_s >> 24);
  buf[3] = (uint8_t)((r.timestamp_s >> 16) & 0xFF);
  buf[4] = (uint8_t)((r.timestamp_s >> 8) & 0xFF);
  buf[5] = (uint8_t)(r.timestamp_s & 0xFF);
  uint16_t crc = crc16_ccitt(buf, 6);
  buf[6] = (uint8_t)(crc >> 8);
  buf[7] = (uint8_t)(crc & 0xFF);

  int addr = recordAddressByIndex(idx);
  eepromWriteBlock(addr, buf, RECORD_SIZE);
  // lectura verificación opcional
  return true;
}

bool readRecordFromEEPROM(int idx, HistRecord &r_out) {
  uint8_t buf[RECORD_SIZE];
  int addr = recordAddressByIndex(idx);
  eepromReadBlock(addr, buf, RECORD_SIZE);

  uint16_t hum = ((uint16_t)buf[0] << 8) | buf[1];
  uint32_t ts = ((uint32_t)buf[2] << 24) | ((uint32_t)buf[3] << 16) | ((uint32_t)buf[4] << 8) | buf[5];
  uint16_t stored_crc = ((uint16_t)buf[6] << 8) | buf[7];

  uint16_t calc = crc16_ccitt(buf, 6);
  if (calc != stored_crc) return false; // CRC mismatch -> registro inválido

  r_out.humedad_x100 = hum;
  r_out.timestamp_s = ts;
  r_out.crc = stored_crc;
  return true;
}

// Borra la estructura de EEPROM (formateo lógico)
void clearEEPROMAll() {
  // Escribimos cabecera nueva y no tocamos obligatoriamente todo byte por byte (opcional)
  header.magic = EEPROM_MAGIC;
  header.version = EEPROM_VERSION;
  header.reserved = 0;
  header.record_count = 0;
  header.write_index = 0;
  writeHeaderToEEPROM();

  // Si quieres, limpiar físicamente EEPROM (es costoso en desgaste):
  // for (int i = EEPROM_RECORDS_START; i < EEPROM.length(); ++i) EEPROM.update(i, 0xFF);
}

// ------------------------ UTILIDADES TIEMPO ------------------------
uint32_t getTimestampSeconds() {
#if USE_RTC
  DateTime now = rtc.now();
  return (uint32_t)now.unixtime();
#else
  // Si no hay RTC, usamos epochOffset_s (usuario debe setearlo si se quiere hora real)
  unsigned long elapsed_s = (millis() - bootMillis) / 1000UL;
  return epochOffset_s + (uint32_t)elapsed_s;
#endif
}

// ------------------------ OPERACIONES PRINCIPALES ------------------------
bool captureReading(float &humidity_out) {
  float h = dht.readHumidity();
  if (isnan(h)) return false;
  humidity_out = h;
  return true;
}

// Graba un registro (usa header.write_index como posición circular)
bool persistRecord(uint16_t humedad_x100, uint32_t timestamp_s) {
  HistRecord r;
  r.humedad_x100 = humedad_x100;
  r.timestamp_s = timestamp_s;
  // crc calculado al escribir
  int idx = header.write_index % EEPROM_MAX_RECORDS;
  if (!writeRecordToEEPROM(idx, r)) return false;

  // actualizar cabecera: incremento puntero y cantidad
  header.write_index = (header.write_index + 1) % EEPROM_MAX_RECORDS;
  if (header.record_count < EEPROM_MAX_RECORDS) header.record_count++;
  writeHeaderToEEPROM();
  return true;
}

// ------------------------ SERIAL CLI ------------------------
void printHelp() {
  Serial.println(F("Comandos (case-insensitive):"));
  Serial.println(F("  leer          -> lee sensor y muestra (no guarda)"));
  Serial.println(F("  guardar       -> lee sensor y guarda en EEPROM"));
  Serial.println(F("  ver           -> muestra registros almacenados (EEPROM)"));
  Serial.println(F("  ver_ram       -> muestra registros en RAM (cache)"));
  Serial.println(F("  export_csv    -> exporta todos los registros en formato CSV (timestamp,humedad)"));
  Serial.println(F("  stats         -> promedio, máximo, mínimo (EEPROM)"));
  Serial.println(F("  clear         -> limpia historial (cabecera)"));
  Serial.println(F("  settime <s>   -> establece epoch seconds (si NO hay RTC)"));
  Serial.println(F("  flush         -> vacía RAM cache a EEPROM (forzar persistencia)"));
  Serial.println(F("  help          -> muestra este texto"));
  Serial.println();
}

void printSingleRecordHuman(const HistRecord &r, int indexNumber) {
  float h = (float)r.humedad_x100 / 100.0;
  Serial.print("Reg #");
  Serial.print(indexNumber);
  Serial.print(" | Humedad: ");
  Serial.print(h, 2);
  Serial.print(" % | Time(s): ");
  Serial.print(r.timestamp_s);
#if USE_RTC
  // si hay RTC, podemos mostrar fecha legible (opcional, requiere RTClib)
  DateTime dt = rtc.now(); // solo ejemplo; para convertir r.timestamp_s -> DateTime se requeriría método
#endif
  Serial.println();
}

// Muestra todos los registros (orden: del más antiguo al más reciente)
void showAllRecords() {
  uint16_t total = header.record_count;
  if (total == 0) {
    Serial.println(F("No hay registros en EEPROM."));
    return;
  }

  int oldestIndex = (header.write_index + EEPROM_MAX_RECORDS - total) % EEPROM_MAX_RECORDS;
  Serial.print(F("Mostrando "));
  Serial.print(total);
  Serial.println(F(" registros (EEPROM):"));

  HistRecord r;
  for (uint16_t i = 0; i < total; ++i) {
    int idx = (oldestIndex + i) % EEPROM_MAX_RECORDS;
    bool ok = readRecordFromEEPROM(idx, r);
    if (!ok) {
      Serial.print("Reg #");
      Serial.print(i + 1);
      Serial.println(" : CRC ERROR or invalid");
    } else {
      printSingleRecordHuman(r, i + 1);
    }
  }
}

// Export CSV
void exportCSVAll() {
  uint16_t total = header.record_count;
  if (total == 0) {
    Serial.println(F("No hay registros para exportar."));
    return;
  }
  Serial.println(F("timestamp_s,humedad_percent"));
  int oldestIndex = (header.write_index + EEPROM_MAX_RECORDS - total) % EEPROM_MAX_RECORDS;
  HistRecord r;
  for (uint16_t i = 0; i < total; ++i) {
    int idx = (oldestIndex + i) % EEPROM_MAX_RECORDS;
    if (readRecordFromEEPROM(idx, r)) {
      Serial.print(r.timestamp_s);
      Serial.print(',');
      float h = (float)r.humedad_x100 / 100.0;
      Serial.println(h, 2);
    } // si CRC falla, saltar
  }
}

// Estadísticas
void printStats() {
  uint16_t total = header.record_count;
  if (total == 0) { Serial.println(F("No hay datos para estadisticas.")); return; }
  int oldestIndex = (header.write_index + EEPROM_MAX_RECORDS - total) % EEPROM_MAX_RECORDS;
  HistRecord r;
  float sum = 0.0;
  float maxv = -9999;
  float minv = 9999;
  int valid = 0;
  for (uint16_t i = 0; i < total; ++i) {
    int idx = (oldestIndex + i) % EEPROM_MAX_RECORDS;
    if (readRecordFromEEPROM(idx, r)) {
      float h = (float)r.humedad_x100 / 100.0;
      sum += h;
      if (h > maxv) maxv = h;
      if (h < minv) minv = h;
      valid++;
    }
  }
  Serial.println(F("=== Estadísticas ==="));
  Serial.print(F("Registros válidos: ")); Serial.println(valid);
  Serial.print(F("Promedio: ")); Serial.print(sum / valid, 2); Serial.println(F(" %"));
  Serial.print(F("Máximo: ")); Serial.print(maxv, 2); Serial.println(F(" %"));
  Serial.print(F("Mínimo: ")); Serial.print(minv, 2); Serial.println(F(" %"));
  Serial.println(F("===================="));
}

// Añade al cache RAM (no persiste automáticamente)
void addToRamCache(const HistRecord &r) {
  if (ramCacheCount < MAX_RAM_CACHE) {
    ramCache[ramCacheCount++] = r;
  } else {
    // shift-left (simple) para mantener window, se puede usar circular si se desea
    for (int i = 1; i < MAX_RAM_CACHE; ++i) ramCache[i - 1] = ramCache[i];
    ramCache[MAX_RAM_CACHE - 1] = r;
  }
}

// Escribe todo RAM cache a EEPROM (persistencia forzada)
void flushRamCacheToEEPROM() {
  for (int i = 0; i < ramCacheCount; ++i) {
    persistRecord(ramCache[i].humedad_x100, ramCache[i].timestamp_s);
  }
  ramCacheCount = 0;
  Serial.println(F("Cache RAM volcado a EEPROM."));
}

// ------------------------ SETUP / LOOP ------------------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(50);
  Serial.println(F("=== Inicializando Historial_Humedad ==="));

  dht.begin();
  bootMillis = millis();

#if USE_RTC
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println(F("ERROR: RTC no encontrado."));
  } else {
    // opcional: sincronizar epochOffset_s desde RTC
    epochOffset_s = (uint32_t)rtc.now().unixtime();
    Serial.print(F("RTC inicializado. Epoch: "));
    Serial.println(epochOffset_s);
  }
#else
  epochOffset_s = 0; // por defecto 0; usuario puede usar settime
#endif

  // calcular espacio y max registros
  EEPROM_RECORDS_START = EEPROM_HEADER_ADDR + EEPROM_HEADER_SIZE;
  EEPROM_MAX_RECORDS = (EEPROM.length() - EEPROM_RECORDS_START) / RECORD_SIZE;
  if (EEPROM_MAX_RECORDS < 1) {
    Serial.println(F("ERROR: EEPROM demasiado pequeña para registros."));
    while (1) delay(1000);
  }
  Serial.print(F("EEPROM capacidad registros: "));
  Serial.println(EEPROM_MAX_RECORDS);

  // inicializar header si es necesario
  initEEPROMHeaderIfNeeded();

  Serial.println(F("Inicialización completa. Escribe 'help' para ver comandos."));
}

void loop() {
  // manejo de comandos serial
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    s.trim();
    s.toLowerCase();

    if (s.equals("help")) {
      printHelp();
    } else if (s.equals("leer")) {
      float h;
      if (!captureReading(h)) Serial.println(F("Error lectura sensor."));
      else {
        Serial.print(F("Lectura: "));
        Serial.print(h, 2);
        Serial.println(F(" %"));
      }
    } else if (s.equals("guardar") || s.equals("save")) {
      float h;
      if (!captureReading(h)) {
        Serial.println(F("Error lectura sensor. No se guardó."));
      } else {
        uint16_t hx100 = (uint16_t)round(h * 100.0);
        uint32_t ts = getTimestampSeconds();
        HistRecord r; r.humedad_x100 = hx100; r.timestamp_s = ts;
        // agregar a RAM cache y persistir opcional (decidir política)
        addToRamCache(r);
        // aquí persistimos inmediatamente para asegurar que el dato no se pierda:
        // persistRecord(hx100, ts);
        Serial.println(F("Lectura añadida a cache RAM. Use 'flush' para persistir en EEPROM."));
      }
    } else if (s.equals("flush")) {
      flushRamCacheToEEPROM();
    } else if (s.equals("ver") || s.equals("view")) {
      showAllRecords();
    } else if (s.equals("ver_ram") || s.equals("view_ram")) {
      if (ramCacheCount == 0) Serial.println(F("RAM cache vacío."));
      else {
        Serial.println(F("Registros en RAM (no persistidos):"));
        for (int i = 0; i < ramCacheCount; ++i) printSingleRecordHuman(ramCache[i], i + 1);
      }
    } else if (s.equals("export_csv") || s.equals("export")) {
      exportCSVAll();
    } else if (s.equals("stats")) {
      printStats();
    } else if (s.startsWith("settime ")) {
      // settime <epoch_seconds>
      String t = s.substring(8);
      t.trim();
      uint32_t sec = (uint32_t)t.toInt();
      epochOffset_s = sec;
      bootMillis = millis();
      Serial.print(F("Epoch offset seteado a: "));
      Serial.println(epochOffset_s);
    } else if (s.equals("clear")) {
      clearEEPROMAll();
      Serial.println(F("EEPROM formateada (cabecera reiniciada)."));
    } else {
      Serial.println(F("Comando desconocido. Escribe 'help'."));
    }
  }

  // opcional: acciones periódicas (ejemplo: volcar cache cada N minutos)
  // NOTA: por defecto no hacemos nada en background para evitar escrituras involuntarias.

  delay(10); // pequeña pausa no bloqueante
}

