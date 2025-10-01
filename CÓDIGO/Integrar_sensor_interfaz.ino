#include <Adafruit_NeoPixel.h>		// Incluye la librería para controlar tiras de LEDs tipo NeoPixel (RGB direccionables)

// =========================
// Definición de constantes (pines utilizados)
// =========================
const int PinLed = 6;       // Pin de la tira LED NeoPixel
const int PinLuz = 7;       // Pin para luces generales
const int PinEnabled = 5;   // Pin de activación de motor (puerta)
const int PinAspersor = 3;  // Pin para el aspersor de agua
const int PinVent = 9;      // Pin para el ventilador
const int PinCale = 4;      // Pin para la calefacción
const int PinInput1 = 10;   // Pin de control motor puerta (dirección 1)
const int PinInput2 = 11;   // Pin de control motor puerta (dirección 2)

// =========================
// Definición de variables
// =========================
char VarEntrada = 0;   // Variable donde se guarda el carácter recibido por Serial
int Contador = 0;      // Contador usado en bucles (control de tiempo con delayMicroseconds)

// =========================
// Creación de objetos
// =========================
Adafruit_NeoPixel led = Adafruit_NeoPixel(4, PinLed, NEO_GRB + NEO_KHZ800);
// Se crea un objeto "led" para controlar 4 LEDs NeoPixel conectados al pin PinLed
// Formato NEO_GRB (verde, rojo, azul) y comunicación a 800 KHz

// =========================
// Configuración inicial (setup)
// =========================
void setup(){
  pinMode(PinLed, OUTPUT);       // Configura el pin de la tira LED como salida
  pinMode(PinLuz, OUTPUT);       // Pin de luces generales como salida
  pinMode(PinEnabled, OUTPUT);   // Pin de habilitación de motor como salida
  pinMode(PinAspersor, OUTPUT);  // Pin del aspersor como salida
  pinMode(PinVent, OUTPUT);      // Pin del ventilador como salida
  pinMode(PinCale, OUTPUT);      // Pin de la calefacción como salida
  pinMode(PinInput1, OUTPUT);    // Pin de dirección 1 del motor como salida
  pinMode(PinInput2, OUTPUT);    // Pin de dirección 2 del motor como salida

  led.begin();				    // Inicializa la tira de LEDs NeoPixel
  Serial.begin(9600);		    // Inicia la comunicación Serial a 9600 baudios
}
 
// =========================
// Bucle principal (loop)
// =========================
void loop(){
	if (Serial.available() > 0) {     // Si hay datos disponibles en el puerto Serial
		VarEntrada = Serial.read();   // Leer el carácter recibido
      	Serial.println(VarEntrada);   // Mostrar el carácter recibido en el monitor serial

      	// Según el carácter recibido, ejecuta una acción
      	switch (VarEntrada) {

      		case 'a': // Encender tira LED en color azul
        		led.setBrightness(100);             	 // Ajusta brillo al 100
  				for(int i = 0; i < 4; i++){		     // Recorre los 4 LEDs
		    		led.setPixelColor(i, 0, 0, 255);   // Asigna color azul a cada LED
    				led.show();		                 // Actualiza la tira LED
            	}
          		break;

          	case 'b': // Apagar tira LED
          		led.setBrightness(80);             	 // Ajusta brillo al 80 (aunque se apaga igual)
  				for(int i = 0; i < 4; i++){		
	    			led.setPixelColor(i, 0, 0, 0);    // Apaga cada LED (color negro = apagado)
    				led.show();			
            	}
          		break;

          	case 'c': // Encender luces generales
          		digitalWrite(PinLuz, HIGH);
 				break;

          	case 'd': // Apagar luces generales
          		digitalWrite(PinLuz, LOW);
 				break;

          	case 'e': // Abrir puerta (sin fin de carrera, usando tiempo fijo)
          		// Dirección del motor para abrir
          		digitalWrite(PinInput1, HIGH);     
        		digitalWrite(PinInput2, LOW);
          		analogWrite(PinEnabled, 255);      // Motor a máxima velocidad
          		for (Contador = 0; Contador < 50; Contador++){
          			delayMicroseconds(8000);        // Espera breve repetida (simula el tiempo de apertura)
        		}
          		analogWrite(PinEnabled, 0);        // Detener motor
          		break;

          	case 'f': // Cerrar puerta (similar al caso anterior, pero dirección inversa)
          		digitalWrite(PinInput1, LOW);     
        		digitalWrite(PinInput2, HIGH);    // Dirección contraria
          		analogWrite(PinEnabled, 255);     // Motor a máxima velocidad
          		for (Contador = 0; Contador < 50; Contador++){
          			delayMicroseconds(8000);        // Tiempo de cierre
                }
                analogWrite(PinEnabled, 0);       // Detener motor
 				break;

          	case 'g': // Encender ventilador
          		analogWrite(PinVent, 255);        // Ventilador al máximo
 				break;

          	case 'h': // Apagar ventilador
          		analogWrite(PinVent, 0);          // Ventilador apagado
 				break;

          	case 'i': // Encender aspersor
          		analogWrite(PinAspersor, 255);    // Aspersor al máximo
 				break;

          	case 'j': // Apagar aspersor
          		analogWrite(PinAspersor, 0);      // Aspersor apagado
 				break;

          	case 'k': // Encender calefacción
          		digitalWrite(PinCale, HIGH);      // Calefacción ON
 				break;

          	case 'l': // Apagar calefacción
          		digitalWrite(PinCale, LOW);       // Calefacción OFF
 				break;
        }
    }      
}
