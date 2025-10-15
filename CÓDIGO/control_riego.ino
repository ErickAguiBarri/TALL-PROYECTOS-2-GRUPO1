// control_riego.ino
// Control automático de bomba de agua según humedad del suelo

int sensorPin = 34;   // pin analógico del sensor de humedad
int relePin = 5;      // pin digital para el módulo relé
int valorHumedad = 0;
int humedadPorc = 0;
int umbral = 40;      // Para simular distintos cultivos(Lechuga 60, Tomate 45, Fresa 70 entre otros)

void setup() {
  Serial.begin(115200);
  pinMode(relePin, OUTPUT);
  digitalWrite(relePin, HIGH); // relé apagado (activo en LOW)
}

void loop() {
  valorHumedad = analogRead(sensorPin);
  humedadPorc = map(valorHumedad, 4095, 0, 0, 100); // convierte lectura a porcentaje

  Serial.print("Humedad: ");
  Serial.print(humedadPorc);
  Serial.println("%");

  if (humedadPorc < umbral) {
    Serial.println("Suelo seco - Encendiendo bomba");
    digitalWrite(relePin, LOW); // activa la bomba
  } else {
    Serial.println("Humedad adecuada - Apagando bomba");
    digitalWrite(relePin, HIGH); // apaga la bomba
  }

  delay(2000);
}
