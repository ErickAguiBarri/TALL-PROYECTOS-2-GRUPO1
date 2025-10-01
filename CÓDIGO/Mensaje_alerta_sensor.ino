#include <DHT.h>              // Incluye la librería para el sensor DHT (temperatura y humedad)
#include <LiquidCrystal.h>    // Incluye la librería para controlar la pantalla LCD

// Configuración del sensor DHT
#define DHT_PIN 2             // El pin digital 2 está conectado al sensor DHT
#define DHT_TYPE DHT22        // Se define que el sensor usado es el modelo DHT22

// Configuración de la pantalla LCD: (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Se inicializa el LCD con los pines donde está conectado

// Pin para el buzzer de alerta
#define BUZZER_PIN 8          // El pin digital 8 será para el buzzer

// Crear objeto del sensor DHT
DHT dht(DHT_PIN, DHT_TYPE);   // Se crea un objeto "dht" con el pin y tipo de sensor

// Variables globales
float temperatura, humedad;   // Aquí se guardarán los valores de temperatura y humedad
int fallosConsecutivos = 0;   // Contador de fallos de lectura seguidos
bool alertaActiva = false;    // Bandera para indicar si la alerta está activa

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(9600);         // Inicializa el puerto serie a 9600 baudios para enviar datos al monitor serie
  lcd.begin(16, 2);           // Configura la pantalla LCD para 16 columnas y 2 filas
  pinMode(BUZZER_PIN, OUTPUT);// Configura el pin del buzzer como salida
  dht.begin();                // Inicia la comunicación con el sensor DHT
  
  lcd.print("Iniciando...");  // Muestra un mensaje de inicio en la pantalla LCD
  delay(2000);                // Espera 2 segundos antes de continuar
}

// -------------------- LOOP --------------------
void loop() {
  // Intentar leer el sensor cada 2 segundos
  if (leerSensor()) {         // Si la lectura es válida...
    mostrarDatosNormal();     // Mostrar los valores de temperatura y humedad
    alertaActiva = false;     // Se desactiva la alerta
    noTone(BUZZER_PIN);       // Se apaga el buzzer (si estaba encendido)
  } else {                    // Si la lectura falla...
    fallosConsecutivos++;     // Aumenta el contador de fallos seguidos
    mostrarAlertaError();     // Muestra en la pantalla que hubo error
    activarAlertaSonora();    // Enciende el buzzer con un tono
    alertaActiva = true;      // Activa la bandera de alerta
  }
  
  delay(2000);                // Espera 2 segundos antes de la siguiente lectura
}

// -------------------- FUNCIONES --------------------

// Función que lee datos del sensor
bool leerSensor() {
  humedad = dht.readHumidity();       // Lee la humedad relativa
  temperatura = dht.readTemperature();// Lee la temperatura en grados Celsius
  
  // Verifica si la lectura es válida (no es NaN = "Not a Number")
  if (!isnan(humedad) && !isnan(temperatura)) {
    fallosConsecutivos = 0;           // Reinicia el contador de fallos
    return true;                      // Devuelve que la lectura fue correcta
  }
  return false;                       // Devuelve que hubo un error
}

// Función para mostrar datos correctos en el LCD
void mostrarDatosNormal() {
  lcd.clear();                        // Limpia la pantalla
  lcd.setCursor(0, 0);                // Pone el cursor en la primera fila
  lcd.print("T:");                    // Muestra "T:" (Temperatura)
  lcd.print(temperatura, 1);          // Imprime la temperatura con 1 decimal
  lcd.print("C H:");                  // Muestra "C H:" (Celsius y Humedad)
  lcd.print(humedad, 0);              // Imprime la humedad sin decimales
  lcd.print("%");                     // Añade el símbolo %
  
  lcd.setCursor(0, 1);                // Pone el cursor en la segunda fila
  lcd.print("Sensor: OK");            // Indica que el sensor funciona bien
}

// Función para mostrar mensaje de error en el LCD y serie
void mostrarAlertaError() {
  lcd.clear();                        // Limpia la pantalla
  lcd.setCursor(0, 0);                // Cursor en primera fila
  lcd.print("ALERTA!");               // Muestra "ALERTA!"
  lcd.setCursor(0, 1);                // Cursor en segunda fila
  lcd.print("Error sensor DHT");      // Muestra que hubo un error en el sensor
  
  Serial.print("Alerta! Fallo #");    // Imprime en el monitor serie
  Serial.println(fallosConsecutivos); // Indica el número de fallos consecutivos
}

// Función para activar el buzzer
void activarAlertaSonora() {
  tone(BUZZER_PIN, 1000, 500);        // Hace sonar el buzzer a 1000 Hz por 500 ms
}
