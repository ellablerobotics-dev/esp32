#include "esp_camera.h"

// =============================================
// Freenove ESP32-S3-WROOM CAM pin configuration
// =============================================

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1

#define XCLK_GPIO_NUM     15
#define SIOD_GPIO_NUM      4
#define SIOC_GPIO_NUM      5

#define Y9_GPIO_NUM       16
#define Y8_GPIO_NUM       17
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       12
#define Y5_GPIO_NUM       10
#define Y4_GPIO_NUM        8
#define Y3_GPIO_NUM        9
#define Y2_GPIO_NUM       11

#define VSYNC_GPIO_NUM     6
#define HREF_GPIO_NUM      7
#define PCLK_GPIO_NUM     13


void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("Freenove ESP32-S3 Camera Test");
  Serial.println("==============================");

  Serial.print("PSRAM: ");
  Serial.println(psramFound() ? "YES" : "NO");


  // =============================================
  // Camera configuration
  // =============================================

  camera_config_t config = {};

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  // Camera data
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  // Camera clock
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;

  // Sync
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;

  // Camera control
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  // =============================================
  // Camera settings
  // =============================================

  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_JPEG;

  // Start with 320 x 240
  config.frame_size = FRAMESIZE_QVGA;

  config.jpeg_quality = 12;

  config.fb_count = 1;

  if (psramFound()) {

    Serial.println("Using PSRAM");

    config.fb_location = CAMERA_FB_IN_PSRAM;

  } else {

    Serial.println("Using internal RAM");

    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;


  // =============================================
  // Initialize camera
  // =============================================

  Serial.println();
  Serial.println("Initializing camera...");

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {

    Serial.printf(
      "Camera initialization FAILED: 0x%08X\n",
      err
    );

    return;
  }

  Serial.println("Camera initialization SUCCESS!");


  // =============================================
  // Display sensor information
  // =============================================

  sensor_t *sensor = esp_camera_sensor_get();

  if (sensor != NULL) {

    Serial.print("Camera PID: 0x");
    Serial.println(sensor->id.PID, HEX);
  }


  // =============================================
  // First test capture
  // =============================================

  Serial.println();
  Serial.println("Taking first picture...");

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {

    Serial.println("Camera capture FAILED!");

    return;
  }

  Serial.println("Camera capture SUCCESS!");

  Serial.print("Width: ");
  Serial.println(fb->width);

  Serial.print("Height: ");
  Serial.println(fb->height);

  Serial.print("JPEG size: ");
  Serial.print(fb->len);
  Serial.println(" bytes");

  esp_camera_fb_return(fb);

  Serial.println();
  Serial.println("Camera ready!");
}


void loop() {

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {

    Serial.println("Capture failed!");

    delay(1000);

    return;
  }

  Serial.printf(
    "Frame: %d x %d | %u bytes\n",
    fb->width,
    fb->height,
    fb->len
  );

  esp_camera_fb_return(fb);

  delay(1000);
}