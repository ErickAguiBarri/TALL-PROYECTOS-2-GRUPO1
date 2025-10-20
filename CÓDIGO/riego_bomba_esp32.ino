// Sprint 3 - Victor
// Conexión sensor de humedad + bomba de riego mediante relé

int pinSensor = 34;  // entrada analógica
int pinRele = 5;     // salida digital para relé
int valorHumedad = 0;
int humedadPorc = 0;
int umbral = 45;     // humedad mínima para activar la bomba

void setup() {
  Serial.begin(115200);
  pinMode(pinRele, OUTPUT);
  digitalWrite(pinRele, HIGH); // Relé inactivo (activo en LOW)
  delay(1000);
  Serial.println("Sistema de riego automático iniciado...");
}

void loop() {
  // Leer sensor de humedad
  valorHumedad = analogRead(pinSensor);
  humedadPorc = map(valorHumedad, 4095, 0, 0, 100);

  Serial.print("Humedad del suelo: ");
  Serial.print(humedadPorc);
  Serial.println("%");

  // Lógica de control
  if (humedadPorc < umbral) {
    Serial.println("→ Suelo seco: encendiendo bomba...");
    digitalWrite(pinRele, LOW); // activa la bomba
  } else {
    Serial.println("→ Suelo húmedo: apagando bomba...");
    digitalWrite(pinRele, HIGH); // apaga la bomba
  }

  delay(3000); // esperar 3 segundos
}
