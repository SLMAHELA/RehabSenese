import paho.mqtt.client as mqtt
import time
import random
import ssl

broker = "3f3e94d58cb04730b7ae589933af0dd2.s1.eu.hivemq.cloud"
port = 8883

username = "RehabSebse"
password = "Aloysius1234"

topic = "rehab/patient1/angle"

client = mqtt.Client()

# 🔐 Secure connection
client.tls_set(tls_version=ssl.PROTOCOL_TLS)
client.username_pw_set(username, password)

client.connect(broker, port, 60)

print("Connected to HiveMQ Cloud...")

while True:
    angle = random.randint(20, 90)
    client.publish(topic, angle)
    print(f"Sent angle: {angle}")
    time.sleep(2)