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

# ===== LOGIC VARIABLES =====
last_angle = 0
rep_count = 0
going_up = False

good_reps = 0
bad_reps = 0
peak_angle = 0


def on_message(client, userdata, msg):
    global last_angle, rep_count, going_up
    global good_reps, bad_reps, peak_angle

    angle = float(msg.payload.decode())
    print(f"Received: {angle}")

    # Track peak angle
    if angle > peak_angle:
        peak_angle = angle

    # Detect upward movement
    if angle > last_angle:
        going_up = True

    # Detect peak → downward movement (rep end)
    if going_up and angle < last_angle:
        rep_count += 1   # ✅ count EVERY rep ONLY ONCE
        
        if peak_angle > 70:
            good_reps += 1
            print(f"✅ GOOD REP: {rep_count}")
        else:
            bad_reps += 1
            print(f"❌ BAD REP")

        going_up = False
        peak_angle = 0

    last_angle = angle

    # ===== WRITE TO INFLUXDB =====

    # Angle data
    angle_point = Point("rehab_angle").field("angle", float(angle))

    # Total reps
    rep_point = Point("rehab_reps").field("count", int(rep_count))

    # Quality data
    quality_good = Point("rehab_quality").field("good", int(good_reps))
    quality_bad = Point("rehab_quality").field("bad", int(bad_reps))

    write_api.write(bucket=bucket, org=org,
                    record=[angle_point, rep_point, quality_good, quality_bad])


# MQTT Client
client = mqtt.Client()
client.tls_set(tls_version=ssl.PROTOCOL_TLS)
client.username_pw_set(username, password)

client.on_message = on_message

client.connect(broker, port, 60)
client.subscribe(topic)

print("Backend connected to HiveMQ...")

client.loop_forever()




