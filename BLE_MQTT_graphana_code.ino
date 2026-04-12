#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// BLE
#include <NimBLEDevice.h>

NimBLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// UUIDs
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-abcdef123456"

MPU6050 mpu1(0x68); // thigh
MPU6050 mpu2(0x69); // shin

float offset = 0;

// REP VARIABLES 
float last_angle = 0;
bool in_motion = false;
bool passed_min = false;
float peak_angle = 0;

int rep_count = 0;
int good_reps = 0;
int bad_reps = 0;

// MQTT 
const char* ssid = "abcnet";
const char* password = "aaaaaaaa";

const char* mqtt_server = "3f3e94d58cb04730b7ae589933af0dd2.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "RehabSebse";
const char* mqtt_pass = "Aloysius1234";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// BLE Callbacks 
class MyServerCallbacks: public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device Connected");
  }

  void onDisconnect(NimBLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device Disconnected");
  }
};

// Angle Function  
float getAngle(MPU6050 &mpu) {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  return atan2(ax, az) * 180 / PI;
}

//  WiFi  
void setup_wifi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  int timeout = 20;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
  } else {
    Serial.println("\nWiFi FAILED (continuing)");
  }
}

//  MQTT reconnect  
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");

    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
    } else {
      Serial.println("failed, retrying...");
      delay(2000);
    }
  }
}

// SETUP 
void setup() {
  Serial.begin(115200);

  espClient.setInsecure();
  client.setBufferSize(128);

  Wire.begin(21, 22);

  mpu1.initialize();
  mpu2.initialize();
  delay(1000); // stability

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

  // ===== WiFi + MQTT =====
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  // ===== BLE =====
  NimBLEDevice::init("RehabSense_Knee");
  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      0x10
                    );

  pCharacteristic->createDescriptor("2902");

  pService->start();
  NimBLEDevice::getAdvertising()->start();

  Serial.println("BLE Started...");
}

// ===== LOOP =====
void loop() {

  if (!client.connected()) reconnect();
  client.loop();

  static float smooth_knee = 0;
  static unsigned long last_mqtt = 0;

  // ===== READ =====
  float thigh_angle = getAngle(mpu1);
  float shin_angle  = getAngle(mpu2);

  float knee_raw = abs(thigh_angle - shin_angle) - offset;
  if (knee_raw < 0) knee_raw = 0;

  // ===== SMOOTH =====
  smooth_knee = 0.8 * smooth_knee + 0.2 * knee_raw;
  float angle = smooth_knee;

  // Clamp
  if (angle < 0) angle = 0;
  if (angle > 120) angle = 120;

  // ===== REP LOGIC =====
  if (!in_motion && angle > 20 && (angle - last_angle) > 1) {
    in_motion = true;
    peak_angle = angle;
  }

  if (in_motion && angle > peak_angle) {
    peak_angle = angle;
  }

  if (in_motion && angle > 50) {
    passed_min = true;
  }

  if (in_motion && angle < 20) {
    if (passed_min) {
      rep_count++;

      if (peak_angle >= 70) {
        good_reps++;
        //Serial.println("GOOD REP");
      } else {
        bad_reps++;
        //Serial.println("BAD REP");
      }
    }

    in_motion = false;
    passed_min = false;
    peak_angle = 0;
  }

  last_angle = angle;

  // ===== BLE =====
  if (true) {
    String value = "Angle:" + String(angle,1) +
                   ",Reps:" + String(rep_count) +
                   ",G:" + String(good_reps) +
                   ",B:" + String(bad_reps);

    pCharacteristic->setValue(value.c_str());
    pCharacteristic->notify();

    Serial.println(value);
  }

  // MQTT (5 Hz) 
  if (millis() - last_mqtt > 200) {
    if (client.connected()) {
      String mqttMsg = "Angle:" + String(angle,1) +
                 ",Reps:" + String(rep_count) +
                 ",G:" + String(good_reps) +
                 ",B:" + String(bad_reps);
        client.publish("rehab/patient1/angle", mqttMsg.c_str());
    }
    last_mqtt = millis();
  }

  delay(50);
}
