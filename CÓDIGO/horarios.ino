/*****************************************************************************************
 * PROYECTO: Sistema Inteligente de Riego y Monitoreo de Invernadero
 * AUTOR: Cristian Luiyi García Huallpa
 * FECHA: Octubre 2025
 * DESCRIPCIÓN:
 *   Sistema automatizado que controla el riego de un invernadero basado en tiempo real
 *   (RTC DS3231) y condiciones ambientales (DHT11 y sensor de humedad de suelo).
 *
 *   Permite:
 *    - Visualización en LCD I2C (20x4)
 *    - Control automático del relé de riego
 *    - Lectura ambiental y decisión lógica inteligente
 *
 * LIBRERÍAS REQUERIDAS:
 *    - <Wire.h>
 *    - <RTClib.h>
 *    - <LiquidCrystal_I2C.h>
 *    - <DHT.h>
 *****************************************************************************************/

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

/*****************************************************************************************
 * --- CONFIGURACIÓN DE HARDWARE ---
 *****************************************************************************************/
#define DHTPIN        2       // Pin digital del sensor DHT11
#define DHTTYPE       DHT11   // Tipo de sensor (DHT11 o DHT22)
#define RELAY_PIN     7       // Salida digital hacia el módulo relé
#define SOIL_PIN      A0      // Entrada analógica del sensor de humedad del suelo

// Inicialización de módulos
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 20, 4);
DHT dht(DHTPIN, DHTTYPE);

/*****************************************************************************************
 * --- ESTRUCTURA DE HORARIOS DE RIEGO ---
 *****************************************************************************************/
struct HorarioRiego {
  int horaInicio;
  int minutoInicio;
  int horaFin;
  int minutoFin;
};

// Intervalos de riego programados
HorarioRiego horarios[] = {
  {6, 0, 6, 5},     // 06:00 - 06:05
  {18, 0, 18, 5}    // 18:00 - 18:05
};
const int NUM_HORARIOS = sizeof(horarios) / sizeof(horarios[0]);

/*****************************************************************************************
 * --- PARÁMETROS DEL SISTEMA ---
 *****************************************************************************************/
const int UMBRAL_HUMEDAD_SUELO = 500; // Valor de humedad del suelo (0-1023)
bool estadoRiego = false;              // Variable de control del relé

/*****************************************************************************************
 * --- CONFIGURACIÓN INICIAL ---
 *****************************************************************************************/
void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  lcd.init();
  lcd.backlight();
  dht.begin();

  if (!rtc.begin()) {
    lcd.print("Error: No se detecta RTC");
    while (1);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistema Invernadero");
  lcd.setCursor(0, 1);
  lcd.print("Inicializando...");
  delay(2000);
}

/*****************************************************************************************
 * --- FUNCIÓN: VERIFICAR HORARIO DE RIEGO ---
 *****************************************************************************************/
bool estaEnHorarioRiego(const DateTime &ahora) {
  for (int i = 0; i < NUM_HORARIOS; i++) {
    if ((ahora.hour() > horarios[i].horaInicio ||
         (ahora.hour() == horarios[i].horaInicio && ahora.minute() >= horarios[i].minutoInicio)) &&
        (ahora.hour() < horarios[i].horaFin ||
         (ahora.hour() == horarios[i].horaFin && ahora.minute() <= horarios[i].minutoFin))) {
      return true;
    }
  }
  return false;
}

/*****************************************************************************************
 * --- FUNCIÓN: CONTROL DE RIEGO ---
 *****************************************************************************************/
void controlarRiego(const DateTime &ahora, int humedadSuelo) {
  bool horarioActivo = estaEnHorarioRiego(ahora);
  if (horarioActivo && humedadSuelo > UMBRAL_HUMEDAD_SUELO) {
    digitalWrite(RELAY_PIN, HIGH);
    estadoRiego = true;
  } else {
    digitalWrite(RELAY_PIN, LOW);
    estadoRiego = false;
  }
}

/*****************************************************************************************
 * --- FUNCIÓN: ACTUALIZAR LCD ---
 *****************************************************************************************/
void mostrarLCD(const DateTime &ahora, float temp, float hum, int humedadSuelo) {
  lcd.clear();

  // Línea 1 - Fecha
  lcd.setCursor(0, 0);
  lcd.print("Fecha: ");
  lcd.print(ahora.day());
  lcd.print("/");
  lcd.print(ahora.month());
  lcd.print("/");
  lcd.print(ahora.year() % 100);

  // Línea 2 - Hora
  lcd.setCursor(0, 1);
  lcd.print("Hora: ");
  if (ahora.hour() < 10) lcd.print("0");
  lcd.print(ahora.hour());
  lcd.print(":");
  if (ahora.minute() < 10) lcd.print("0");
  lcd.print(ahora.minute());

  // Línea 3 - Datos ambientales
  lcd.setCursor(0, 2);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print("C H:");
  lcd.print(hum, 0);
  lcd.print("%");

  // Línea 4 - Estado de riego
  lcd.setCursor(0, 3);
  if (estadoRiego)
    lcd.print("RIEGO ACTIVADO ");
  else
    lcd.print("RIEGO APAGADO  ");
}

/*****************************************************************************************
 * --- BUCLE PRINCIPAL ---
 *****************************************************************************************/
void loop() {
  DateTime ahora = rtc.now();
  float temperatura = dht.readTemperature();
  float humedadAire = dht.readHumidity();
  int humedadSuelo = analogRead(SOIL_PIN);

  // Validar lecturas del DHT
  if (isnan(temperatura) || isnan(humedadAire)) {
    lcd.clear();
    lcd.print("Error en sensor DHT");
    delay(2000);
    return;
  }

  // Control de riego basado en condiciones
  controlarRiego(ahora, humedadSuelo);

  // Mostrar en LCD
  mostrarLCD(ahora, temperatura, humedadAire, humedadSuelo);

  delay(2000);
}
