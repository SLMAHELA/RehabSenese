# RehabSense – Smart Wearable Knee Rehabilitation Tracker

---

## 🔍 Overview

RehabSense is a low-cost IoT-based wearable system designed to monitor knee rehabilitation exercises in real-time. It uses dual IMU sensors to track knee angle, count repetitions, and evaluate exercise quality. The system provides continuous data to support home-based physiotherapy and improve recovery outcomes.

---

## ⚠️ Problem Statement

Musculoskeletal injuries and post-surgical rehabilitation (e.g., ACL reconstruction, knee replacement) require consistent physiotherapy. However:

- Patients often perform exercises incorrectly at home  
- Clinicians lack objective movement data  
- Progress tracking is subjective and inconsistent  
- Traditional tools (e.g., goniometer) cannot measure continuously  

There is a need for an affordable, real-time monitoring system.

---

## 🎯 Project Objectives

The system is designed to:

- Measure knee joint angle using two IMU sensors  
- Calculate range of motion  
- Detect and count physiotherapy repetitions  
- Classify good vs bad movements  
- Stream data wirelessly  
- Visualize rehabilitation progress  

---

## 🏗️ System Architecture
IMU (Thigh) + IMU (Shin) ► ESP32 MCU ► Moblie via BLE and MQTT (HiveMQ) Via WIFI ► Python Backend ► InfluxDB ► Grafana Dashboard

---

## 📐 Angle Calculation
Knee Angle ≈ Thigh Pitch – Shin Pitch

---

---

## ⚙️ Features

- Real-time knee angle measurement  
- Automatic repetition counting  
- Good vs bad rep classification  
- BLE data streaming  
- MQTT communication  
- Grafana dashboard visualization  
- Local CSV backup logging  

---

## 🧰 Hardware Components

### Main Unit (Thigh Strap)
- ESP32 microcontroller  
- MPU6050 IMU  
- Li-ion battery (2500–3000mAh)  
- TP4056 charging module  
- MT3608 boost converter  
- Power switch  
- Custom 3D printed enclosure  

### Secondary Unit (Shin Strap)
- MPU6050 IMU  
- I2C wired connection  
- Compact enclosure  

---

## 💻 Software & Tools

- Arduino IDE  
- Python (backend.py)  
- HiveMQ (MQTT broker)  
- InfluxDB (time-series database)  
- Grafana (dashboard visualization)  

---

## 🚀 Setup Instructions

### ESP32
1. Open Arduino IDE  
2. Select ESP32 Dev Module  
3. Upload the code  

### Backend
pip install -r requirements.txt
python backend.py


### InfluxDB
- Create bucket: rehab_data  
- Set token, org, and URL in backend.py  

### Grafana
- Connect InfluxDB as data source  
- Create dashboard panels  


---

## 📊 Results

- Real-time knee angle visualization  
- Repetition tracking  
- Exercise quality classification  
- Dashboard-based monitoring  

---

## 🌱 Relevance & Sustainability

- Supports remote rehabilitation  
- Reduces clinic visits  
- Low-cost reusable electronics  
- Improves long-term recovery tracking  

---

## 🔮 Future Improvements

- Mobile app integration  
- AI-based movement analysis  
- Cloud deployment  
- Remote doctor monitoring  

---

## 👤 Author

Vimukthi Weerakkodi Mohottige Don  
BME project


