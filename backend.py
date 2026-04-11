import paho.mqtt.client as mqtt
import ssl
from influxdb_client import InfluxDBClient, Point

# CONFIG (use ENV in real deployment)
broker = "3f3e94d58cb04730b7ae589933af0dd2.s1.eu.hivemq.cloud"
port = 8883
username = "RehabSebse"
password = "Aloysius1234"

topic = "rehab/patient1/angle"

# InfluxDB Cloud
url = "https://eu-central-1-1.aws.cloud2.influxdata.com"
token = "Xd8pjCsTx0sjDRGZwKGyRZBx4TJQ9FP6H1QJhPvc4RekErhsDMXruAnubLnpx8rtxyhpwp0DG_3yY2fZxa2UvQ=="
org = "RehabSense"
bucket = "rehab_data"

client_influx = InfluxDBClient(url=url, token=token, org=org)
write_api = client_influx.write_api()

def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print("Received:", payload)

    try:
        parts = payload.split(",")

        angle = float(parts[0].split(":")[1])
        reps  = int(parts[1].split(":")[1])
        good  = int(parts[2].split(":")[1])
        bad   = int(parts[3].split(":")[1])

        write_api.write(bucket=bucket, org=org, record=[
            Point("rehab_angle").field("angle", angle),
            Point("rehab_reps").field("count", reps),
            Point("rehab_quality").field("good", good),
            Point("rehab_quality").field("bad", bad)
        ])

    except Exception as e:
        print("Parse error:", e)

client = mqtt.Client()
client.tls_set(tls_version=ssl.PROTOCOL_TLS)
client.username_pw_set(username, password)

client.on_message = on_message

client.connect(broker, port, 60)
client.subscribe(topic)

print("Cloud backend running...")

client.loop_forever()