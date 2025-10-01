#include <DHT.h>
#include <LiquidCrystal.h>

// Configuración del sensor DHT
#define DHT_PIN 2
#define DHT_TYPE DHT22   // Cambia a DHT11 si usas ese modelo

// Configuración de la pantalla LCD (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Crear objeto DHT
DHT dht(DHT_PIN, DHT_TYPE);

// Variables para almacenar lecturas
float temperatura;
float humedad;
float indiceCalor;

// Variables para control de actualización
unsigned long ultimaLectura = 0;
const unsigned long intervaloLectura = 2000; // Leer cada 2 segundos

// Variables para animación
int frame = 0;
unsigned long ultimoFrame = 0;

void setup() {
  // Inicializar comunicación serial
  Serial.begin(9600);
  
  // Inicializar LCD (16 columnas, 2 filas)
  lcd.begin(16, 2);
  
  // Inicializar sensor DHT
  dht.begin();
  
  // Mostrar pantalla de inicio
  mostrarPantallaInicio();
  
  Serial.println("=== SISTEMA DHT22 + LCD ===");
  Serial.println("Esperando lecturas...");
}

void loop() {
  unsigned long tiempoActual = millis();
  
  // Leer sensor cada intervalo definido
  if (tiempoActual - ultimaLectura >= intervaloLectura) {
    leerSensor();
    ultimaLectura = tiempoActual;
  }
  
  // Actualizar animación cada 500ms
  if (tiempoActual - ultimoFrame >= 500) {
    actualizarAnimacion();
    ultimoFrame = tiempoActual;
  }
  
  // Mostrar datos en LCD
  mostrarDatosLCD();
}

void leerSensor() {
  // Leer humedad y temperatura
  humedad = dht.readHumidity();
  temperatura = dht.readTemperature(); // Leer en Celsius
  
  // Verificar si la lectura fue exitosa
  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error: No se pudo leer el sensor DHT");
    
    // Mostrar error en LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error sensor DHT");
    lcd.setCursor(0, 1);
    lcd.print("Verificar conexion");
    return;
  }
  
  // Calcular índice de calor
  indiceCalor = dht.computeHeatIndex(temperatura, humedad, false);
  
  // Mostrar datos por serial
  mostrarDatosSerial();
}

void mostrarDatosLCD() {
  lcd.clear();
  
  // Línea 1: Temperatura
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperatura, 1);
  lcd.print((char)223); // Símbolo de grados
  lcd.print("C");
  
  // Icono de temperatura según valor
  lcd.setCursor(14, 0);
  if (temperatura < 15) {
    lcd.print("❄"); // Frío
  } else if (temperatura >= 15 && temperatura <= 28) {
    lcd.print("☀"); // Normal
  } else {
    lcd.print("🔥"); // Caliente
  }
  
  // Línea 2: Humedad
  lcd.setCursor(0, 1);
  lcd.print("Humedad: ");
  lcd.print(humedad, 0);
  lcd.print("%");
  
  // Icono de humedad según valor
  lcd.setCursor(14, 1);
  if (humedad < 30) {
    lcd.print("☁"); // Seco
  } else if (humedad >= 30 && humedad <= 70) {
    lcd.print("⛅"); // Normal
  } else {
    lcd.print("💧"); // Húmedo
  }
}

void mostrarDatosSerial() {
  Serial.print("T: ");
  Serial.print(temperatura, 1);
  Serial.print("°C | H: ");
  Serial.print(humedad, 0);
  Serial.print("% | IC: ");
  Serial.print(indiceCalor, 1);
  Serial.print("°C | Estado: ");
  
  if (humedad < 30) {
    Serial.println("Muy seco");
  } else if (humedad >= 30 && humedad <= 60) {
    Serial.println("Confortable");
  } else {
    Serial.println("Húmedo");
  }
}

void mostrarPantallaInicio() {
  lcd.setCursor(0, 0);
  lcd.print("  SISTEMA DHT22  ");
  lcd.setCursor(0, 1);
  lcd.print("  INICIANDO...  ");
  delay(3000);
  lcd.clear();
}

void actualizarAnimacion() {
  // Esta función puede usarse para animaciones simples
  frame = (frame + 1) % 4;
}