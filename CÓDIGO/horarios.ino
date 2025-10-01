C#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

// Inicializar RTC y LCD
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Estructura para los horarios
struct Horario {
  int dia;           // 1=Lunes ... 5=Viernes
  int horaInicio;
  int minutoInicio;
  int horaFin;
  int minutoFin;
  const char* materia;
};

// Definimos los horarios de la semana
Horario horarios[] = {
  {1, 8, 0, 10, 0, "Matematicas"},
  {2, 9, 0, 11, 0, "Historia"},
  {3, 10, 0, 12, 0, "Ingles"},
  {4, 14, 0, 16, 0, "Programacion"},
  {5, 7, 0, 9, 0, "Fisica"}
};

int numHorarios = sizeof(horarios) / sizeof(horarios[0]);

void setup() {
  lcd.init();
  lcd.backlight();
  if (!rtc.begin()) {
    lcd.print("No se detecta RTC!");
    while (1);
  }
  lcd.setCursor(0,0);
  lcd.print("Sistema Horarios");
  delay(2000);
}

void loop() {
  DateTime now = rtc.now();
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Fecha: ");
  lcd.print(now.day());
  lcd.print("/");
  lcd.print(now.month());

  lcd.setCursor(0,1);
  lcd.print("Hora: ");
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());

  // Buscar horario actual
  bool encontrado = false;
  for (int i = 0; i < numHorarios; i++) {
    if (horarios[i].dia == now.dayOfTheWeek() && 
        (now.hour() > horarios[i].horaInicio || 
        (now.hour() == horarios[i].horaInicio && now.minute() >= horarios[i].minutoInicio)) &&
        (now.hour() < horarios[i].horaFin || 
        (now.hour() == horarios[i].horaFin && now.minute() <= horarios[i].minutoFin))) {
      
      lcd.setCursor(0,2);
      lcd.print("Clase: ");
      lcd.print(horarios[i].materia);
      encontrado = true;
      break;
    }
  }

  if (!encontrado) {
    lcd.setCursor(0,2);
    lcd.print("Sin clase ahora");
  }

  delay(5000); // actualizar cada 5 seg
}
ÓDIGO
