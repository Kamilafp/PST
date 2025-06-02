#include <Arduino.h>
#include <Servo.h>

#define analogPin A0
#define digitalPin 4

// Deklarasi 2 servo
Servo servo1;
Servo servo2;

void setup() {
  Serial.begin(9600);
  pinMode(digitalPin, INPUT);
  // Kalibrasi servo dengan rentang mikrodetik (500-2500µs)
  servo1.attach(D1, 500, 2500);  // Servo 1 di pin D1
  servo2.attach(D3, 500, 2500);  // Servo 2 di pin D2
  
  // Inisialisasi posisi tengah (opsional)
  servo1.writeMicroseconds(1500);  // 90°
  servo2.writeMicroseconds(1500);  // 90°
  delay(1000);
}

void loop() {
  int analogValue = analogRead(analogPin);

  Serial.print("Analog Value: ");
  Serial.print(analogValue);

  if (analogValue < 500) {
    Serial.println(" => RAIN DETECTED");
  } else {
    Serial.println(" => NO RAIN");
  }

  delay(500); 

  // Gerakan servo1: 0° → 180°
  servo1.writeMicroseconds(500);    // 0°
  servo2.writeMicroseconds(2500);   // Servo 2 tetap 180°
  delay(2000);
  
  // Gerakan servo2: 180° → 0°
  servo1.writeMicroseconds(2500);   // Servo 1 tetap 180°
  servo2.writeMicroseconds(500);    // 0°
  delay(2000);
}
