#include <MKS_SERVO_D.h>

MKS_SERVO_D servo;

const uint8_t slaveAddress = 1;
uint8_t direction = 0;

void setup() {
  Serial.begin(115200);
  Serial1.begin(38400);
  servo.initialize(&Serial1);
}

void loop() {
  int32_t position = 0;
  int16_t speed = 0;

  if (servo.readPosition(slaveAddress, position)) {
    Serial.print("position = ");
    Serial.println(position);
  }

  if (servo.readSpeed(slaveAddress, speed)) {
    Serial.print("speed = ");
    Serial.print(speed);
    Serial.println(" RPM");
  }

  MKS_SERVO_D::AckStatus status = servo.runPositionMode1(slaveAddress, direction, 100, 200, 32000);
  if (status == MKS_SERVO_D::ACK_STARTED) {
    status = servo.waitForAck(0);
    if (status == MKS_SERVO_D::ACK_COMPLETED) {
      direction ^= 1;
    }
  }

  delay(3000);
}

