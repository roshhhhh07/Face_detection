from flask import Flask, request, send_file, send_from_directory, jsonify
from flask_cors import CORS
import face_recognition
import numpy as np
import cv2
import os
import time
import requests

app = Flask(__name__)
CORS(app)

# ── Folders ──────────────────────────────────────────────────────────────────
os.makedirs("unknown_faces", exist_ok=True)
os.makedirs("known_faces", exist_ok=True)

# ── Config ───────────────────────────────────────────────────────────────────
ESP32_IP = "http://10.243.213.161"
PC_IP    = "192.168.1.34"
PORT     = 5000

# ── Load known face encodings ─────────────────────────────────────────────────
known_encodings = []
image_files = [f"R{i}.jpg" for i in range(1, 28)]

for file in image_files:
    try:
        img = face_recognition.load_image_file(file)
        enc = face_recognition.face_encodings(img)
        if enc:
            known_encodings.append(enc[0])
            print(f"✅ Loaded {file}")
        else:
            print(f"⚠️  No face found in {file}")
    except Exception as e:
        print(f"❌ Error loading {file}: {e}")

print(f"\n📦 Total known encodings loaded: {len(known_encodings)}\n")


# ── Helper ────────────────────────────────────────────────────────────────────
def _trigger(url, label):
    try:
        requests.get(url, timeout=3)
        print(f"✅ {label} triggered")
    except Exception as e:
        print(f"⚠️  {label} request failed: {e}")


# ── Routes ────────────────────────────────────────────────────────────────────

@app.route('/')
def dashboard():
    return send_file('Dashboard.html')


@app.route('/upload', methods=['POST'])
def upload():
    file = request.data

    if not file:
        print("❌ Empty request received")
        return "UNKNOWN", 400

    # Decode image
    nparr = np.frombuffer(file, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

    if img is None:
        print("❌ Failed to decode image")
        return "UNKNOWN", 400

    # Resize only if too large
    h, w = img.shape[:2]
    if w > 640:
        img = cv2.resize(img, (640, 480))
    print(f"📐 Image shape: {img.shape}")

    timestamp = int(time.time())
    rgb_img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    # Detect faces
    face_locations = face_recognition.face_locations(rgb_img)
    encodings = face_recognition.face_encodings(rgb_img, face_locations)

    # No face detected
    if len(encodings) == 0:
        print("❌ No face detected")
        cv2.imwrite(f"unknown_faces/unknown_{timestamp}.jpg", img)
        _trigger(f"{ESP32_IP}/buzzer", "Buzzer")
        return "UNKNOWN"

    # No known faces loaded
    if len(known_encodings) == 0:
        print("⚠️  No known encodings loaded — treating as unknown")
        cv2.imwrite(f"unknown_faces/unknown_{timestamp}.jpg", img)
        _trigger(f"{ESP32_IP}/buzzer", "Buzzer")
        return "UNKNOWN"

    # Compare against known faces
    for encoding in encodings:
        distances = face_recognition.face_distance(known_encodings, encoding)
        best_index = np.argmin(distances)
        best_distance = distances[best_index]
        print(f"🔍 Best match distance: {best_distance:.4f}")

        if best_distance < 0.6:
            cv2.imwrite(f"known_faces/known_{timestamp}.jpg", img)
            print(f"✅ KNOWN PERSON detected — distance: {best_distance:.4f}")
            _trigger(f"{ESP32_IP}/unlock", "Unlock")
            return "KNOWN"

    # Face detected but no match
    cv2.imwrite(f"unknown_faces/unknown_{timestamp}.jpg", img)
    print(f"🚨 UNKNOWN PERSON detected")
    _trigger(f"{ESP32_IP}/buzzer", "Buzzer")
    return "UNKNOWN"


@app.route('/dashboard-data')
def dashboard_data():
    logs = []
    faces = []

    # Scan known_faces folder
    for f in os.listdir("known_faces"):
        if f.endswith(".jpg"):
            try:
                ts = int(f.replace("known_", "").replace(".jpg", ""))
                faces.append({"type": "known", "filename": f, "ts": ts})
                logs.append({"type": "known",  "ts": ts})
                logs.append({"type": "unlock", "ts": ts})
                logs.append({"type": "lock",   "ts": ts + 5})
            except:
                pass

    # Scan unknown_faces folder
    for f in os.listdir("unknown_faces"):
        if f.endswith(".jpg"):
            try:
                ts = int(f.replace("unknown_", "").replace(".jpg", ""))
                faces.append({"type": "unknown", "filename": f, "ts": ts})
                logs.append({"type": "unknown", "ts": ts})
            except:
                pass

    logs.sort(key=lambda x: x["ts"])
    faces.sort(key=lambda x: x["ts"])

    return jsonify({"logs": logs, "faces": faces, "door_locked": True})


@app.route('/image/<folder>/<filename>')
def serve_image(folder, filename):
    return send_from_directory(f"{folder}_faces", filename)


# ── Run ───────────────────────────────────────────────────────────────────────
if __name__ == '__main__':
    print(f"\n🚀 Server running at http://{PC_IP}:{PORT}")
    print(f"📱 Open on phone: http://{PC_IP}:{PORT}\n")
    app.run(host='0.0.0.0', port=PORT, debug=False)
