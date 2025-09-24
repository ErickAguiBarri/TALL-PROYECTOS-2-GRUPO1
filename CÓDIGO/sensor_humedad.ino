int pinHumedad = 34;  // Pin analógico en ESP32
int valorHumedad = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Leer valor analógico (0 - 4095 en ESP32)
  valorHumedad = analogRead(pinHumedad);

  // Convertir a porcentaje (aproximado, depende de calibración)
  int humedadPorc = map(valorHumedad, 4095, 0, 0, 100);

  Serial.print("Valor crudo: ");
  Serial.print(valorHumedad);
  Serial.print("  |  Humedad: ");
  Serial.print(humedadPorc);
  Serial.println("%");

  delay(1000);
}
