#include <Wire.h>
#include <MPU6050.h>

// ===== BLE =====
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// UUIDs
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-abcdef123456"

// ===== MPU6050 =====
MPU6050 mpu1(0x68); // thigh
MPU6050 mpu2(0x69); // shin

float offset = 0;

// ===== REP VARIABLES =====
float last_angle = 0;
bool in_motion = false;
bool passed_min = false;
float peak_angle = 0;

int rep_count = 0;
int good_reps = 0;
int bad_reps = 0;

// ===== BLE Callbacks =====
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device Connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device Disconnected");
  }
};

// ===== Angle Function =====
float getAngle(MPU6050 &mpu) {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  return atan2(ax, az) * 180 / PI;
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  mpu1.initialize();
  mpu2.initialize();

  Serial.println("MPU system ready");

  // ===== CALIBRATION =====
  float sum = 0;
  for (int i = 0; i < 50; i++) {
    float t = getAngle(mpu1);
    float s = getAngle(mpu2);
    sum += abs(t - s);
    delay(20);
  }

  offset = sum / 50;

  Serial.println("AUTO CALIBRATED");
  Serial.print("Offset: ");
  Serial.println(offset);

  // ===== BLE =====
  BLEDevice::init("RehabSense_Knee");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  Serial.println("BLE Started...");
}

// ===== LOOP =====
void loop() {

  static float smooth_knee = 0;

  // ===== READ ANGLES =====
  float thigh_angle = getAngle(mpu1);
  float shin_angle  = getAngle(mpu2);

  float knee_raw = abs(thigh_angle - shin_angle) - offset;
  if (knee_raw < 0) knee_raw = 0;

  // ===== SMOOTHING =====
  smooth_knee = 0.8 * smooth_knee + 0.2 * knee_raw;
  float angle = smooth_knee;
  //==max limit==
  if (angle < 0) angle = 0;
  if (angle > 120) angle = 120;

  // ===== REP LOGIC =====

  // Start movement (above 20° and rising)
  if (!in_motion && angle > 20 && angle > last_angle) {
    in_motion = true;
    peak_angle = angle;
  }

  // Track peak
  if (in_motion) {
    if (angle > peak_angle) {
      peak_angle = angle;
    }
  }

  // Check minimum threshold (50°)
  if (in_motion && angle > 50) {
    passed_min = true;
  }

  // Rep completion (back below 20°)
  if (in_motion && angle < 20) {

    if (passed_min) {
      rep_count++;

      if (peak_angle >= 70) {
        good_reps++;
        Serial.println("GOOD REP");
      } else {
        bad_reps++;
        Serial.println("BAD REP");
      }
    }

    // Reset
    in_motion = false;
    passed_min = false;
    peak_angle = 0;
  }

  last_angle = angle;

  // ===== BLE SEND =====
  if (deviceConnected) {
    String value = "Angle:" + String(angle,1) +
                   ",Reps:" + String(rep_count) +
                   ",Good:" + String(good_reps) +
                   ",Bad:" + String(bad_reps);

    pCharacteristic->setValue(value.c_str());
    pCharacteristic->notify();

    Serial.println(value);
  }

  delay(100);
}