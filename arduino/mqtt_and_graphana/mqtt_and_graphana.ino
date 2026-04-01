#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MPU6050.h>

// WiFi
const char* ssid = "abcnet";
const char* password = "aaaaaaaa";

// HiveMQ
const char* mqtt_server = "3f3e94d58cb04730b7ae589933af0dd2.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "RehabSebse";
const char* mqtt_pass = "Aloysius1234";

// MQTT client
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Sensors
MPU6050 thigh(0x68);
MPU6050 shin(0x69);

float baseOffset = 0;

float getAngle(MPU6050 &mpu) {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  return atan2(ax, az) * 180 / PI;
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      // connected
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  thigh.initialize();
  shin.initialize();

  delay(2000);

  // calibration
  float sum = 0;

  for (int i = 0; i < 50; i++) {
    sum += abs(getAngle(thigh) - getAngle(shin));
    delay(20);
  }

  baseOffset = sum / 50;

  setup_wifi();

  espClient.setInsecure(); // TLS
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  static float smooth_knee = 0;

  float knee_raw = abs(getAngle(thigh) - getAngle(shin)) - baseOffset;
  if (knee_raw < 0) knee_raw = 0;

  // ✅ smoothing
  smooth_knee = 0.8 * smooth_knee + 0.2 * knee_raw;

  float knee = smooth_knee;

  char msg[10];
  dtostrf(knee, 1, 2, msg);

  client.publish("rehab/patient1/angle", msg);

  Serial.println(knee);

  delay(200);
}