#include <Arduino.h>
#include <DHT.h>
#include <Servo.h>

// Konfigurasi DHT11
#define DHTPIN 4         // D2 = GPIO4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Servo
#define SERVO1_PIN 5     // D1 = GPIO5
#define SERVO2_PIN 0     // D3 = GPIO0

Servo servo1;
Servo servo2;

bool isHot = false;

void setup() {
  Serial.begin(9600);
  dht.begin();

  servo1.attach(SERVO1_PIN, 500, 2500);
  servo2.attach(SERVO2_PIN, 500, 2500);

  // Posisi awal
  servo1.writeMicroseconds(700);   
  servo2.writeMicroseconds(700);
  delay(1000);
}

void loop() {
  float temperature = dht.readTemperature();

  if (isnan(temperature)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return;
  }

  Serial.print("Suhu: ");
  Serial.print(temperature);
  Serial.println(" °C");

  if (temperature >= 29.0) {
    if (!isHot) {
      servo1.writeMicroseconds(2500);
      servo2.writeMicroseconds(2500);
      isHot = true;
      Serial.println(">>> Suhu panas: Servo menutup kanopi");
    }
  } else {
    if (isHot) {
      servo1.writeMicroseconds(700);
      servo2.writeMicroseconds(700);
      isHot = false;
      Serial.println(">>> Suhu normal: Servo membuka kanopi");
    }
  }

  delay(2000);
}