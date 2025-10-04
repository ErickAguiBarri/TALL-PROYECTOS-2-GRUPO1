/***************************************************************
 * PROYECTO:  Sistema de Monitoreo Ambiental - Nodo de Humedad
 * AUTOR:     Cristian Luiyi García Huallpa
 * FECHA:     Octubre 2025
 * PLATAFORMA: Arduino UNO (ATmega328P - 16 MHz, 2 KB SRAM, 32 KB Flash)
 * 
 * DESCRIPCIÓN TÉCNICA GENERAL:
 * Este firmware implementa un sistema de adquisición de datos
 * ambientales a nivel de nodo IoT. El microcontrolador obtiene
 * lecturas de humedad y temperatura desde un sensor DHT11, realiza
 * validaciones de integridad, y transmite la información hacia un
 * sistema superior (Python, servidor web o interfaz gráfica).
 * 
 * El código sigue un enfoque modular y documentado:
 *   1️⃣ Inicialización de hardware y librerías.
 *   2️⃣ Lectura periódica del sensor mediante control de tiempo no bloqueante.
 *   3️⃣ Validación de datos (detección de fallos de lectura).
 *   4️⃣ Formateo estructurado y transmisión serial.
 * 
 * NOTA: Este código está preparado para integrarse fácilmente con:
 *       - Interfaz Tkinter en Python.
 *       - Comunicación vía MQTT o HTTP en futuras versiones.
 ***************************************************************/

// =============================================================
// 1. INCLUSIÓN DE LIBRERÍAS Y CONFIGURACIÓN DEL SENSOR
// =============================================================
#include <DHT.h>  // Librería estándar que controla sensores DHTxx mediante temporización precisa.

// --- Definiciones de pines y tipo de sensor ---
#define DHTPIN 2        // Pin digital al que se conecta la línea de datos del sensor.
#define DHTTYPE DHT11   // Tipo de sensor (puede cambiarse a DHT22 si se usa otro modelo).

// --- Instanciación del objeto DHT ---
DHT dht(DHTPIN, DHTTYPE);  
// Este objeto encapsula las funciones de inicialización y lectura del sensor.
// Internamente, maneja la sincronización del pulso de 18 ms y la decodificación
// del tren de bits que representa humedad y temperatura.

// =============================================================
// 2. DECLARACIÓN DE VARIABLES GLOBALES Y PARÁMETROS DEL SISTEMA
// =============================================================

// Variables de almacenamiento temporal (volátiles por si se usan en ISR).
float humedad = 0.0;        // Valor actual de humedad relativa (%)
float temperatura = 0.0;    // Valor actual de temperatura (°C)

// Variables de control de tiempo (en milisegundos)
unsigned long tiempoAnterior = 0;        // Guarda el último instante de lectura
const unsigned long INTERVALO_LECTURA = 2000; // Intervalo de muestreo (2 segundos)

// =============================================================
// 3. CONFIGURACIÓN INICIAL DEL MICROCONTROLADOR
// =============================================================
void setup() {
  // Inicializa la comunicación serial para transmisión de datos
  Serial.begin(9600);  
  // Configura la UART del ATmega328P en modo 8N1 (8 bits de datos, sin paridad, 1 bit de stop)
  // La velocidad de 9600 bps es suficiente para lecturas periódicas de baja frecuencia.

  // Inicializa el sensor DHT
  dht.begin();  

  // Mensajes iniciales enviados al host para verificar conexión
  Serial.println(F("=== Sistema de Monitoreo Ambiental - Nodo de Humedad ==="));
  Serial.println(F("Inicializando sensor DHT11..."));
  delay(2000);  // Delay inicial necesario para la calibración interna del sensor DHT.
  Serial.println(F("Sensor DHT11 operativo."));
}

// =============================================================
// 4. BUCLE PRINCIPAL DE ADQUISICIÓN Y PROCESAMIENTO
// =============================================================
void loop() {
  // Se obtiene el tiempo actual desde el inicio del programa.
  unsigned long tiempoActual = millis();

  // Control de tiempo no bloqueante: se ejecuta solo cada INTERVALO_LECTURA milisegundos.
  if (tiempoActual - tiempoAnterior >= INTERVALO_LECTURA) {

    // Actualiza el marcador de tiempo de la última lectura
    tiempoAnterior = tiempoActual;

    // =============================================================
    // 4.1 LECTURA DE HUMEDAD Y TEMPERATURA DESDE EL SENSOR
    // =============================================================
    humedad = dht.readHumidity();        // Retorna humedad relativa (%)
    temperatura = dht.readTemperature(); // Retorna temperatura (°C)

    // =============================================================
    // 4.2 VALIDACIÓN DE LECTURA (DETECCIÓN DE NAN)
    // =============================================================
    if (isnan(humedad) || isnan(temperatura)) {
      /*
       * El método readHumidity() y readTemperature() pueden devolver NaN (Not a Number)
       * si el sensor no responde correctamente, hay ruido eléctrico, o la conexión
       * física está incompleta.
       */
      Serial.println(F("ERROR: Lectura inválida del sensor DHT11."));
      return;  // Se interrumpe la iteración para no transmitir datos corruptos.
    }

    // =============================================================
    // 4.3 PROCESAMIENTO DE DATOS (FORMATEO ESTRUCTURADO)
    // =============================================================
    /*
     * Se envían los datos por el puerto serial en un formato legible y estructurado.
     * Esto permite que programas externos (Python, Node-RED, MATLAB, etc.)
     * puedan parsear fácilmente las lecturas.
     *
     * Ejemplo de salida:
     *   HUMEDAD:48.50% | TEMP:23.12°C
     */

    Serial.print(F("HUMEDAD:"));
    Serial.print(humedad, 2);    // Se imprime con dos decimales de precisión.
    Serial.print(F("% | "));     // Separador visual para facilitar parsing.
    Serial.print(F("TEMP:"));
    Serial.print(temperatura, 2);
    Serial.println(F("°C"));

    // =============================================================
    // 4.4 ANÁLISIS DE CONDICIONES (opcional)
    // =============================================================
    /*
     * En sistemas de control ambiental, aquí podría agregarse lógica condicional:
     * 
     * if (humedad < 30) {
     *     Serial.println("ALERTA: Humedad baja detectada.");
     *     digitalWrite(RELE_PIN, HIGH); // Activar humidificador, por ejemplo.
     * }
     */
  }

  // El resto del tiempo el microcontrolador permanece en espera activa
  // (se podría entrar en modo de bajo consumo si el sistema lo requiere).
}

