// sensor_humedad_serial.ino (ESP32)
int pinHumedad = 34;  // pin analógico en ESP32
int valorHumedad = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("INICIO_SENSOR");
}

void loop() {
  valorHumedad = analogRead(pinHumedad); // 0 - 4095 en ESP32
  int humedadPorc = map(valorHumedad, 4095, 0, 0, 100);
  unsigned long ts = millis();
  // Formato: TS,RAW,HUM
  Serial.print(ts);
  Serial.print(",");
  Serial.print(valorHumedad);
  Serial.print(",");
  Serial.println(humedadPorc);
  delay(1000); // 1 s
}
