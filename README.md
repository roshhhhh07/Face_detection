#  Face Recognition Smart Lock System

A real-time **face recognition-based smart lock system** that combines computer vision and IoT.
The system detects and recognizes faces using a camera and controls a **solenoid lock via ESP32** for authorized access.

---

##  Features

*  Real-time face detection and recognition
*  Automatic door unlock for authorized users
*  Buzzer alert for unknown users
*  Saves images of known and unknown faces
*  Web dashboard for logs and monitoring
*  ESP32 integration for hardware control

---

##  Tech Stack

* **Language:** Python
* **Libraries:**

  * OpenCV (image processing)
  * face_recognition (face encoding & matching)
  * Flask (backend server)
  * NumPy
* **Hardware:**

  * ESP32
  * Solenoid lock
  * Buzzer
* **Frontend:** HTML Dashboard

---

##  How It Works

1. Camera captures image and sends it to the server
2. Server processes image using face detection
3. Face is converted into encoding (numerical vector)
4. Encoding is compared with stored known faces
5. If match found:

   * Door unlocks via ESP32
6. If no match:

   * Buzzer is triggered
   * Image saved as unknown

---

## Project Structure

```
├── known_faces/          # Stored images of known users
├── unknown_faces/        # Images of unknown detections
├── Dashboard.html        # Frontend dashboard
├── main.py               # Flask server + ML logic
├── esp32_code/           # ESP32 control code
└── README.md
```

---

## 🧪 Setup Instructions

### 1. Clone the repository

```bash
git clone https://github.com/your-username/your-repo-name.git
cd your-repo-name
```

### 2. Install dependencies

```bash
pip install opencv-python face_recognition flask flask-cors numpy requests
```

### 3. Add known faces

* Add images as `R1.jpg, R2.jpg...` in the root folder

### 4. Update IP addresses

```python
ESP32_IP = "http://your-esp32-ip"
PC_IP = "your-local-ip"
```

### 5. Run server

```bash
python main.py
```



##  Dashboard

* Displays logs of:

  * Known detections
  * Unknown detections
  * Lock/unlock events
* Shows saved images from system

---

##  Applications

* Smart home security
* Office access control
* Attendance systems
* IoT-based automation

---

## What I Learned

* Face encoding and matching logic
* Real-time image processing
* Backend server handling using Flask
* Integrating ML with hardware (ESP32)
* Building a complete end-to-end system

---

##  Future Improvements

* Improve accuracy in low light
* Add multiple user profiles
* Deploy system on Raspberry Pi
* Add mobile notifications

---

##  Author

* Your Name

---

##  If you like this project, consider giving it a star!
