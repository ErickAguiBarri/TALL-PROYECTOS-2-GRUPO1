#include <DHT.h>              // Incluye la librería para el sensor DHT (temperatura y humedad)
#include <LiquidCrystal.h>    // Incluye la librería para controlar la pantalla LCD

#define DHT_PIN 2             // Pin digital 2 conectado al sensor DHT
#define DHT_TYPE DHT22        // Tipo de sensor (DHT22). Cambiar a DHT11 si es necesario

LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Inicializa el LCD con los pines RS=12, E=11, D4=5, D5=4, D6=3, D7=2
DHT dht(DHT_PIN, DHT_TYPE);            // Crea el objeto del sensor DHT en el pin definido

float temperatura;            // Variable para almacenar la temperatura
float humedad;                // Variable para almacenar la humedad
float indiceCalor;            // Variable para almacenar el índice de calor

unsigned long ultimaLectura = 0;                // Guarda el tiempo de la última lectura
const unsigned long intervaloLectura = 2000;    // Intervalo de lectura del sensor (2 segundos)

int frame = 0;                // Variable para el frame de animación
unsigned long ultimoFrame = 0;// Guarda el tiempo del último cambio de frame

void setup() {
  Serial.begin(9600);         // Inicia comunicación serie a 9600 baudios
  lcd.begin(16, 2);           // Inicializa la pantalla LCD con 16 columnas y 2 filas
  dht.begin();                // Inicia el sensor DHT
  mostrarPantallaInicio();    // Muestra pantalla de inicio en el LCD
  Serial.println("=== SISTEMA DHT22 + LCD ==="); // Mensaje en monitor serie
  Serial.println("Esperando lecturas...");
}

void loop() {
  unsigned long tiempoActual = millis();  // Obtiene el tiempo actual desde que inició Arduino
  
  if (tiempoActual - ultimaLectura >= intervaloLectura) { // Si pasaron 2 segundos desde la última lectura
    leerSensor();              // Lee el sensor
    ultimaLectura = tiempoActual; // Actualiza el tiempo de última lectura
  }
  
  if (tiempoActual - ultimoFrame >= 500) { // Si pasaron 500ms desde el último cambio de frame
    actualizarAnimacion();     // Actualiza animación
    ultimoFrame = tiempoActual;// Actualiza el tiempo de animación
  }
  
  mostrarDatosLCD();           // Muestra los datos en la pantalla LCD
}

void leerSensor() {
  humedad = dht.readHumidity();        // Lee la humedad relativa
  temperatura = dht.readTemperature(); // Lee la temperatura en grados Celsius
  
  if (isnan(humedad) || isnan(temperatura)) { // Si ocurre error en la lectura
    Serial.println("Error: No se pudo leer el sensor DHT");
    lcd.clear();                       // Limpia pantalla LCD
    lcd.setCursor(0, 0); lcd.print("Error sensor DHT"); // Muestra mensaje de error
    lcd.setCursor(0, 1); lcd.print("Verificar conexion");
    return;                            // Sale de la función
  }
  
  indiceCalor = dht.computeHeatIndex(temperatura, humedad, false); // Calcula el índice de calor
  mostrarDatosSerial();                // Muestra los datos en el monitor serie
}

void mostrarDatosLCD() {
  lcd.clear();                         // Limpia pantalla LCD
  
  lcd.setCursor(0, 0);                 // Primera fila
  lcd.print("Temp: "); lcd.print(temperatura, 1); // Imprime temperatura con 1 decimal
  lcd.print((char)223); lcd.print("C"); // Imprime símbolo de grados °C
  
  lcd.setCursor(14, 0);                // Posición para ícono de temperatura
  if (temperatura < 15) lcd.print("❄");      // Frío
  else if (temperatura >= 15 && temperatura <= 28) lcd.print("Normal"); // Normal
  else lcd.print("calor");                // Caliente
  
  lcd.setCursor(0, 1);                 // Segunda fila
  lcd.print("Humedad: "); lcd.print(humedad, 0); lcd.print("%"); // Humedad sin decimales
  
  lcd.setCursor(14, 1);                // Posición para ícono de humedad
  if (humedad < 30) lcd.print("Seco");    // Seco
  else if (humedad >= 30 && humedad <= 70) lcd.print("Normal"); // Normal
  else lcd.print("humedo");                 // Húmedo
}

void mostrarDatosSerial() {
  Serial.print("T: "); Serial.print(temperatura, 1); Serial.print("°C | ");
  Serial.print("H: "); Serial.print(humedad, 0); Serial.print("% | ");
  Serial.print("IC: "); Serial.print(indiceCalor, 1); Serial.print("°C | Estado: ");
  
  if (humedad < 30) Serial.println("Muy seco");   // Clasifica la humedad
  else if (humedad >= 30 && humedad <= 60) Serial.println("Confortable");
  else Serial.println("Húmedo");
}

void mostrarPantallaInicio() {
  lcd.setCursor(0, 0); lcd.print("  SISTEMA DHT22  "); // Mensaje en la primera fila
  lcd.setCursor(0, 1); lcd.print("  INICIANDO...  ");  // Mensaje en la segunda fila
  delay(3000); lcd.clear();                            // Espera 3s y limpia la pantalla
}

void actualizarAnimacion() {
  frame = (frame + 1) % 4;     // Cambia el frame de animación en bucle (0 a 3)
}
