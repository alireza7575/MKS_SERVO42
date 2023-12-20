#include "MKS_SERVO42.h"

MKS_SERVO42 stepper;

void setup() {
  Serial.begin(115200);
  while (!Serial.available())
    delay(10);
  Serial1.begin(38400);
  stepper.initialize(&Serial1);
}

void loop() {
   //Serial.println(stepper.ping(0));
   //delay(500);
   Serial.println(stepper.getCurrentPosition(0));
   delay(500);
}
