#include <Arduino.h>
#include <Servo.h>

Servo myservo;

void setup() {
  myservo.attach(D1);
}

void loop() {
  myservo.write(90);
  delay(2000);
  myservo.write(0);
  delay(2000);
  myservo.write(90);
  delay(2000);
  // for (int pos = 0; pos <= 180; pos++)
  // {
  //   myservo.write(pos);
  //   delay(10);
  // }
  // for (int pos = 180; pos >= 0; pos--)
  // {
  //   myservo.write(pos);
  //   delay(10);
  // }
}
