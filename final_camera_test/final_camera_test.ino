#include "esp_camera.h"
#include <WiFi.h>

// Select camera model
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

const char* ssid = "Nexus";
const char* password = "roshan@0501";

void startCameraServer();

void setup() {

  // FIX 2 — Max CPU speed, reduces lag significantly
  setCpuFrequencyMhz(240);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    // FIX 1 — QVGA is the sweet spot for AI-Thinker, smooth 20+ FPS
    config.frame_size   = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count     = 2;
    // FIX 3 — Always grab the latest frame, eliminates frozen/laggy feel
    config.grab_mode    = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size   = FRAMESIZE_QVGA;  // QVGA even without PSRAM
    config.jpeg_quality = 15;
    config.fb_count     = 1;
    config.grab_mode    = CAMERA_GRAB_LATEST;
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t* s = esp_camera_sensor_get();

  // OV3660 sensor corrections
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }

  // FIX 1 — Lock frame size to QVGA after init as well
  s->set_framesize(s, FRAMESIZE_QVGA);

  // Optional image tuning — improves quality in low light
  s->set_whitebal(s, 1);       // Auto white balance on
  s->set_awb_gain(s, 1);       // AWB gain on
  s->set_exposure_ctrl(s, 1);  // Auto exposure on
  s->set_aec2(s, 1);           // AEC DSP on

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  // FIX 4 — Check WiFi signal strength before connecting
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  // FIX 4 — Print RSSI so you know your signal quality
  Serial.print("Signal strength (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  if (WiFi.RSSI() < -70) {
    Serial.println("WARNING: Weak WiFi signal — move ESP32-CAM closer to router!");
    Serial.println("This is likely causing your lag.");
  } else {
    Serial.println("WiFi signal: Good");
  }

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  // Reduced from 10000ms — keeps watchdog timer happy
  // without blocking the camera server unnecessarily
  delay(1);
}
