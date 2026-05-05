#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

#define MOTION_SENSOR_PIN 21
#define BUZZER_PIN 22
#define RELAY_PIN 23

const char* ssid = "Xd";
const char* password = "12345678";
const char* camIP = "10.243.213.60";

WebServer server(80);

int motionStateCurrent = LOW;
int motionStatePrevious = LOW;

unsigned long unlockStartTime = 0;
unsigned long lastMotionTime = 0;
bool isUnlocked = false;

void handleUnlock() {
  Serial.println("🔥 /unlock endpoint HIT");
  digitalWrite(RELAY_PIN, LOW);  // LOW = relay ON = solenoid powered = UNLOCKED
  Serial.println("🔓 Door UNLOCKED");
  isUnlocked = true;
  unlockStartTime = millis();
  server.send(200, "text/plain", "Unlocked");
}

void handleBuzzer() {
  Serial.println("🔥 /buzzer endpoint HIT");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(2000);
  digitalWrite(BUZZER_PIN, LOW);
  server.send(200, "text/plain", "Buzzer ON");
}

void triggerCamera() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected!");
    return;
  }

  String url = String("http://") + camIP + "/capture";
  Serial.print("🔗 Hitting URL: ");
  Serial.println(url);

  HTTPClient http;
  http.begin(url);
  http.setTimeout(8000);

  int httpCode = http.GET();
  Serial.print("HTTP Code: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    Serial.println("📸 Camera triggered successfully");
  } else {
    Serial.print("❌ Camera trigger FAILED: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
}

void setup() {
  Serial.begin(115200);

  pinMode(MOTION_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY_PIN, HIGH); // HIGH = relay OFF = solenoid no power = LOCKED

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on("/unlock", handleUnlock);
  server.on("/buzzer", handleBuzzer);
  server.begin();

  Serial.println("🚀 Ready!");
}

void loop() {
  server.handleClient();

  motionStatePrevious = motionStateCurrent;
  motionStateCurrent = digitalRead(MOTION_SENSOR_PIN);

  if (motionStatePrevious == LOW && motionStateCurrent == HIGH) {
    if (millis() - lastMotionTime > 5000) {
      Serial.println("👀 Motion detected → Triggering camera");
      lastMotionTime = millis();
      triggerCamera();
    }
  }

  // Auto relock after 5 seconds
  if (isUnlocked && millis() - unlockStartTime > 5000) {
    digitalWrite(RELAY_PIN, HIGH); // HIGH = relay OFF = solenoid loses power = LOCKED
    Serial.println("🔒 Door LOCKED again");
    isUnlocked = false;
  }
}