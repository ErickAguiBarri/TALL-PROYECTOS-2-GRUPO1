#include <DHT.h>
#include <LiquidCrystal.h>

// Configuración del sensor DHT
#define DHT_PIN 2
#define DHT_TYPE DHT22

// Configuración LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Pin para buzzer de alerta
#define BUZZER_PIN 8

DHT dht(DHT_PIN, DHT_TYPE);

float temperatura, humedad;
int fallosConsecutivos = 0;
bool alertaActiva = false;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(BUZZER_PIN, OUTPUT);
  dht.begin();
  
  lcd.print("Iniciando...");
  delay(2000);
}

void loop() {
  // Leer sensor cada 2 segundos
  if (leerSensor()) {
    // Lectura exitosa
    mostrarDatosNormal();
    alertaActiva = false;
    noTone(BUZZER_PIN); // Apagar alerta
  } else {
    // Error en lectura
    fallosConsecutivos++;
    mostrarAlertaError();
    activarAlertaSonora();
    alertaActiva = true;
  }
  
  delay(2000);
}

bool leerSensor() {
  humedad = dht.readHumidity();
  temperatura = dht.readTemperature();
  
  // Si la lectura es exitosa, resetear contador
  if (!isnan(humedad) && !isnan(temperatura)) {
    fallosConsecutivos = 0;
    return true;
  }
  return false;
}

void mostrarDatosNormal() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperatura, 1);
  lcd.print("C H:");
  lcd.print(humedad, 0);
  lcd.print("%");
  
  lcd.setCursor(0, 1);
  lcd.print("Sensor: OK");
}

void mostrarAlertaError() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ALERTA!");
  lcd.setCursor(0, 1);
  lcd.print("Error sensor DHT");
  
  Serial.print("Alerta! Fallo #");
  Serial.println(fallosConsecutivos);
}

void activarAlertaSonora() {
  // Tono de alerta intermitente
  tone(BUZZER_PIN, 1000, 500);
}