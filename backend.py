import paho.mqtt.client as mqtt
import ssl
from influxdb_client import InfluxDBClient, Point

from flask import Flask
import threading

# ================= MQTT CONFIG =================
broker = "3f3e94d58cb04730b7ae589933af0dd2.s1.eu.hivemq.cloud"
port = 8883
username = "RehabSebse"
password = "Aloysius1234"

topic = "rehab/patient1/angle"

# ================= INFLUXDB CONFIG =================
url = "https://eu-central-1-1.aws.cloud2.influxdata.com"
token = "Xd8pjCsTx0sjDRGZwKGyRZBx4TJQ9FP6H1QJhPvc4RekErhsDMXruAnubLnpx8rtxyhpwp0DG_3yY2fZxa2UvQ=="
org = "RehabSense"
bucket = "rehab_data"

client_influx = InfluxDBClient(url=url, token=token, org=org)
write_api = client_influx.write_api()

# ================= MQTT CALLBACK =================
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

# ================= MQTT SETUP =================
client = mqtt.Client()
client.tls_set(tls_version=ssl.PROTOCOL_TLS)
client.username_pw_set(username, password)
client.on_message = on_message

def start_mqtt():
    client.connect(broker, port, 60)
    client.subscribe(topic)
    print("MQTT connected and subscribed...")
    client.loop_forever()

# ================= FLASK (for Render FREE) =================
app = Flask(__name__)

@app.route("/")
def home():
    return "RehabSense backend running 🚀"

# ================= MAIN =================
if __name__ == "__main__":
    # Start MQTT in background thread
    threading.Thread(target=start_mqtt).start()

    # Start web server (required for Render)
    app.run(host="0.0.0.0", port=10000)
