#include <WiFi.h>
#include "esp_camera.h"
#include "esp_http_server.h"

// =========================
// WiFi
// =========================
const char* ssid = "kids24";
const char* password = "happynewyear";

// =========================
// Freenove ESP32-S3-WROOM CAM
// =========================
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1

#define XCLK_GPIO_NUM    15
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5

#define Y2_GPIO_NUM      11
#define Y3_GPIO_NUM       9
#define Y4_GPIO_NUM       8
#define Y5_GPIO_NUM      10
#define Y6_GPIO_NUM      12
#define Y7_GPIO_NUM      18
#define Y8_GPIO_NUM      17
#define Y9_GPIO_NUM      16

#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM    13

httpd_handle_t server = NULL;

// =========================
// Home page
// =========================
static esp_err_t index_handler(httpd_req_t *req)
{
  const char* html =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>ESP32-S3 Camera</title>"
    "</head>"
    "<body style='text-align:center;font-family:Arial;'>"
    "<h2>Freenove ESP32-S3 Camera</h2>"
    "<img src='/stream' style='width:90%;max-width:800px;'>"
    "</body>"
    "</html>";

  httpd_resp_set_type(req, "text/html");

  return httpd_resp_send(
    req,
    html,
    strlen(html)
  );
}

// =========================
// MJPEG stream
// =========================
static esp_err_t stream_handler(httpd_req_t *req)
{
  esp_err_t res = ESP_OK;

  httpd_resp_set_type(
    req,
    "multipart/x-mixed-replace;boundary=frame"
  );

  while (true)
  {
    camera_fb_t* fb = esp_camera_fb_get();

    if (!fb)
    {
      Serial.println("Camera capture failed");
      return ESP_FAIL;
    }

    // Make sure frame is JPEG
    if (fb->format != PIXFORMAT_JPEG)
    {
      esp_camera_fb_return(fb);
      Serial.println("Frame is not JPEG");
      return ESP_FAIL;
    }

    char header[64];

    int header_len = snprintf(
      header,
      sizeof(header),
      "Content-Type: image/jpeg\r\n"
      "Content-Length: %u\r\n\r\n",
      fb->len
    );

    // Boundary
    res = httpd_resp_send_chunk(
      req,
      "\r\n--frame\r\n",
      strlen("\r\n--frame\r\n")
    );

    // JPEG header
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(
        req,
        header,
        header_len
      );
    }

    // JPEG image
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(
        req,
        (const char*)fb->buf,
        fb->len
      );
    }

    esp_camera_fb_return(fb);

    if (res != ESP_OK)
    {
      break;
    }

    delay(10);
  }

  return res;
}

// =========================
// Single snapshot
// =========================
static esp_err_t capture_handler(httpd_req_t *req)
{
  camera_fb_t* fb = esp_camera_fb_get();

  if (!fb)
  {
    Serial.println("Capture failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");

  esp_err_t res = httpd_resp_send(
    req,
    (const char*)fb->buf,
    fb->len
  );

  esp_camera_fb_return(fb);

  return res;
}

// =========================
// Start HTTP server
// =========================
void startCameraServer()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  config.server_port = 80;

  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
  };

  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  httpd_uri_t capture_uri = {
    .uri = "/capture",
    .method = HTTP_GET,
    .handler = capture_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&server, &config) == ESP_OK)
  {
    httpd_register_uri_handler(server, &index_uri);
    httpd_register_uri_handler(server, &stream_uri);
    httpd_register_uri_handler(server, &capture_uri);

    Serial.println("HTTP server started");
  }
  else
  {
    Serial.println("HTTP server failed");
  }
}

// =========================
// Setup
// =========================
void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("=============================");
  Serial.println("Freenove ESP32-S3 Camera");
  Serial.println("=============================");

  Serial.print("PSRAM: ");
  Serial.println(psramFound() ? "YES" : "NO");

  // =========================
  // Camera configuration
  // =========================
  camera_config_t config = {};

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;

  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  // Camera clock
  config.xclk_freq_hz = 20000000;

  // MJPEG streaming needs JPEG
  config.pixel_format = PIXFORMAT_JPEG;

  // Start with 320x240
  config.frame_size = FRAMESIZE_QVGA;

  // Lower number = better quality / bigger file
  config.jpeg_quality = 12;

  if (psramFound())
  {
    config.fb_location = CAMERA_FB_IN_PSRAM;

    // Two buffers usually gives smoother streaming
    config.fb_count = 2;

    config.grab_mode = CAMERA_GRAB_LATEST;
  }
  else
  {
    config.fb_location = CAMERA_FB_IN_DRAM;

    config.fb_count = 1;

    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  }

  // =========================
  // Initialize camera
  // =========================
  Serial.println("Initializing camera...");

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK)
  {
    Serial.printf(
      "Camera initialization failed: 0x%08X\n",
      err
    );

    return;
  }

  Serial.println("Camera initialized successfully");

  // =========================
  // Connect WiFi
  // =========================
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");

  // =========================
  // Start web server
  // =========================
  startCameraServer();

  IPAddress ip = WiFi.localIP();

  Serial.println();
  Serial.println("=============================");
  Serial.println("CAMERA READY");
  Serial.println("=============================");

  Serial.print("IP address: ");
  Serial.println(ip);

  Serial.print("Web page: http://");
  Serial.println(ip);

  Serial.print("Video stream: http://");
  Serial.print(ip);
  Serial.println("/stream");

  Serial.print("Snapshot: http://");
  Serial.print(ip);
  Serial.println("/capture");

  Serial.println("=============================");
}

void loop()
{
  delay(1000);
}