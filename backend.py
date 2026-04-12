import paho.mqtt.client as mqtt
import ssl
from influxdb_client import InfluxDBClient, Point

# MQTT CONFIG
broker = "3f3e94d58cb04730b7ae589933af0dd2.s1.eu.hivemq.cloud"
port = 8883
username = "RehabSebse"
password = "Aloysius1234"

topic = "rehab/patient1/angle"

# InfluxDB CONFIG
url = "http://localhost:8086"
token = "VXppg374SlPoiX7nfz5GSYOdQyxxzMkF-figqb85kSYy_JxHZZv9nDd7UZz4KCVx_w00tryc-zs6TzlW5UTs5w=="
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

        print(f"Angle={angle}, Reps={reps}, Good={good}, Bad={bad}")

        # ===== WRITE TO INFLUXDB =====
        angle_point = Point("rehab_angle").field("angle", angle)
        rep_point   = Point("rehab_reps").field("count", reps)
        good_point  = Point("rehab_quality").field("good", good)
        bad_point   = Point("rehab_quality").field("bad", bad)

        write_api.write(
            bucket=bucket,
            org=org,
            record=[angle_point, rep_point, good_point, bad_point]
        )

    except Exception as e:
        print("Parse error:", e)


# MQTT Client
client = mqtt.Client()
client.tls_set(tls_version=ssl.PROTOCOL_TLS)
client.username_pw_set(username, password)

client.on_message = on_message

client.connect(broker, port, 60)
client.subscribe(topic)

print("Backend connected to HiveMQ...")

client.loop_forever()