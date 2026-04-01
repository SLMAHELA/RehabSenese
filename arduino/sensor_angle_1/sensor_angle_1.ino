#include <Wire.h>
#include <MPU6050.h>

MPU6050 thigh(0x68);
MPU6050 shin(0x69);

float baseOffset = 0;

void setup() {
  Wire.begin(21, 22);
  Serial.begin(115200);

  thigh.initialize();
  shin.initialize();

  delay(2000); // settle sensors

  // capture baseline (leg straight)
  float t = getAngle(thigh);
  float s = getAngle(shin);

  baseOffset = abs(t - s);

  Serial.println("Calibrated!");
}

float getAngle(MPU6050 &mpu) {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  return atan2(ax, az) * 180 / PI;
}

void loop() {
  float thighAngle = getAngle(thigh);
  float shinAngle = getAngle(shin);

  float kneeAngle = abs(thighAngle - shinAngle) - baseOffset;

  if (kneeAngle < 0) kneeAngle = 0;

  Serial.print("Knee Angle: ");
  Serial.println(kneeAngle);

  delay(200);
}