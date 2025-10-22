// Pruebas de programación correctas: sensor + bomba + serial

int sensorPin = 34;  // pin del sensor de humedad
int relePin = 5;     // pin del módulo relé
int valorHumedad = 0;
int humedadPorc = 0;
int umbral = 45;     // umbral mínimo
bool bombaEncendida = false;

void setup() {
  Serial.begin(115200);
  pinMode(relePin, OUTPUT);
  digitalWrite(relePin, HIGH); // bomba apagada
  Serial.println("=== Iniciando pruebas de programación ===");
}

void loop() {
  valorHumedad = analogRead(sensorPin);
  humedadPorc = map(valorHumedad, 4095, 0, 0, 100);

  Serial.print("Lectura: ");
  Serial.print(humedadPorc);
  Serial.print("% | Estado bomba: ");
  Serial.println(bombaEncendida ? "ENCENDIDA" : "APAGADA");

  // Lógica
  if (humedadPorc < umbral && !bombaEncendida) {
    digitalWrite(relePin, LOW);  // activa bomba
    bombaEncendida = true;
    Serial.println("→ Bomba encendida (suelo seco)");
  }
  else if (humedadPorc >= umbral && bombaEncendida) {
    digitalWrite(relePin, HIGH); // apaga bomba
    bombaEncendida = false;
    Serial.println("→ Bomba apagada (suelo húmedo)");
  }

  delay(3000);
}
